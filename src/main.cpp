#include <fstream>
#include <iostream>

#include "Ast.h"
#include "Compiler.h"
#include "Fiber.h"
#include "MagpieString.h"
#include "Object.h"
#include "Parser.h"
#include "VM.h"

using namespace magpie;
using namespace std;


// Reads a file from the given path into a String.
gc<String> readFile(const char* path)
{
  ifstream stream(path);

  if (stream.fail())
  {
    cout << "Could not open file '" << path << "'." << endl;
    return gc<String>();
  }

  // From: http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring.
  string str;

  // Allocate a std::string big enough for the file.
  stream.seekg(0, ios::end);
  str.reserve(stream.tellg());
  stream.seekg(0, ios::beg);

  // Read it in.
  str.assign((istreambuf_iterator<char>(stream)),
             istreambuf_iterator<char>());

  return String::create(str.c_str());
}

int main(int argc, char * const argv[])
{
  if (argc < 2)
  {
    // TODO(bob): Show usage, etc.
    std::cout << "magpie <script>" << std::endl;
    return 1;
  }
  
  // TODO(bob): Hack temp!
  VM vm;
  
  // Read the file.
  const char* fileName = argv[1];
  gc<String> source = readFile(fileName);
  bool success = vm.loadModule(fileName, source);
  
  return success ? 0 : 1;
}
