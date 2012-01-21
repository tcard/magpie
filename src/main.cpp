#include <fstream>
#include <iostream>

#include "Chunk.h"
#include "Fiber.h"
#include "GC.h"
#include "VM.h"
#include "MagpieString.h"
#include "Node.h"

using namespace magpie;

// Reads a file from the given path into a String.
temp<String> readFile(const char* path)
{
  std::ifstream stream(path);

  if (stream.fail())
  {
    std::cout << "Could not open file '" << path << "'." << std::endl;
    return temp<String>();
  }

  // From: http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring.
  std::string str;

  // Allocate a std::string big enough for the file.
  stream.seekg(0, std::ios::end);
  str.reserve(stream.tellg());
  stream.seekg(0, std::ios::beg);

  // Read it in.
  str.assign((std::istreambuf_iterator<char>(stream)),
             std::istreambuf_iterator<char>());

  return String::create(str.c_str());
}

int main(int argc, char * const argv[])
{
  std::cout << "Magpie!\n";

  // TODO(bob): Hack temp!
  VM vm;
  AllocScope scope;

  // Try lexing a file.
  temp<String> source = readFile("../../example/Fibonacci.mag");
  Lexer lexer(source);
  while (true)
  {
    AllocScope tokenScope;
    temp<Token> token = lexer.readToken();
    std::cout << token << std::endl;
    if (token->type() == TOKEN_EOF) break;
  }
  
  // Try making some nodes.
  temp<Node> one = NumberNode::create(1.0);
  temp<Node> two = NumberNode::create(2.0);
  temp<Node> plus = BinaryOpNode::create(one, TOKEN_PLUS, two);
  
  std::cout << plus << std::endl;
  
  gc<Fiber>& fiber = vm.fiber();
  gc<Chunk> return3 = gc<Chunk>(new Chunk(1));
  unsigned short three = fiber->addLiteral(Object::create(3.0));
  return3->write(MAKE_LITERAL(three, 0));
  return3->write(MAKE_RETURN(0));

  gc<Object> return3Method = gc<Object>(new Multimethod(return3));
  unsigned short method = fiber->addLiteral(return3Method);

  gc<Chunk> chunk = gc<Chunk>(new Chunk(3));
  unsigned short zero = fiber->addLiteral(Object::create(0));
  chunk->write(MAKE_LITERAL(zero, 0));
  chunk->write(MAKE_LITERAL(method, 1));
  chunk->write(MAKE_CALL(0, 1, 2));
  chunk->write(MAKE_MOVE(2, 0));
  chunk->write(MAKE_HACK_PRINT(0));
  chunk->write(MAKE_RETURN(0));

  fiber->interpret(chunk);

  return 0;
}
