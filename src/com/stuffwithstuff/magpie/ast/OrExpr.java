package com.stuffwithstuff.magpie.ast;

import com.stuffwithstuff.magpie.parser.Position;

public class OrExpr extends Expr {
  public OrExpr(Position position, Expr left, Expr right) {
    super(position);
    mLeft = left;
    mRight = right;
  }
  
  public Expr getLeft() { return mLeft; }
  public Expr getRight() { return mRight; }
  
  @Override
  public <R, C> R accept(ExprVisitor<R, C> visitor, C context) {
    return visitor.visit(this, context);
  }

  @Override
  public void toString(StringBuilder builder, String indent) {
    mLeft.toString(builder, indent);
    builder.append(" or ");
    mRight.toString(builder, indent);
  }

  private final Expr mLeft;
  private final Expr mRight;
}