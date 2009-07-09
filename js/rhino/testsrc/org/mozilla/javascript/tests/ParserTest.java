package org.mozilla.javascript.tests;

import org.mozilla.javascript.ast.*;

import org.mozilla.javascript.CompilerEnvirons;
import org.mozilla.javascript.Parser;
import org.mozilla.javascript.Token;
import org.mozilla.javascript.testing.TestErrorReporter;

import junit.framework.TestCase;

import java.io.IOException;
import java.io.StringReader;
import java.util.List;

public class ParserTest extends TestCase {

    public void testLinenoAssign() {
        AstRoot root = parse("\n\na = b");
        ExpressionStatement st = (ExpressionStatement) root.getFirstChild();
        AstNode n = st.getExpression();

        assertTrue(n instanceof Assignment);
        assertEquals(Token.ASSIGN, n.getType());
        assertEquals(2, n.getLineno());
    }

    public void testLinenoCall() {
        AstRoot root = parse("\nfoo(123);");
        ExpressionStatement st = (ExpressionStatement) root.getFirstChild();
        AstNode n = st.getExpression();

        assertTrue(n instanceof FunctionCall);
        assertEquals(Token.CALL, n.getType());
        assertEquals(1, n.getLineno());
    }

    public void testLinenoGetProp() {
        AstRoot root = parse("\nfoo.bar");
        ExpressionStatement st = (ExpressionStatement) root.getFirstChild();
        AstNode n = st.getExpression();

        assertTrue(n instanceof PropertyGet);
        assertEquals(Token.GETPROP, n.getType());
        assertEquals(1, n.getLineno());

        PropertyGet getprop = (PropertyGet) n;
        AstNode m = getprop.getRight();

        assertTrue(m instanceof Name);
        assertEquals(Token.NAME, m.getType()); // used to be Token.STRING!
        assertEquals(1, m.getLineno());
    }

    public void testLinenoGetElem() {
        AstRoot root = parse("\nfoo[123]");
        ExpressionStatement st = (ExpressionStatement) root.getFirstChild();
        AstNode n = st.getExpression();

        assertTrue(n instanceof ElementGet);
        assertEquals(Token.GETELEM, n.getType());
        assertEquals(1, n.getLineno());
    }

    public void testLinenoComment() {
        AstRoot root = parse("\n/** a */");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals(1, root.getComments().first().getLineno());
    }

    public void testLinenoComment2() {
        AstRoot root = parse("\n/**\n\n a */");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals(1, root.getComments().first().getLineno());
    }

    public void testLinenoComment3() {
        AstRoot root = parse("\n  \n\n/**\n\n a */");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals(3, root.getComments().first().getLineno());
    }

    public void testLinenoComment4() {
        AstRoot root = parse("\n  \n\n  /**\n\n a */");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals(3, root.getComments().first().getLineno());
    }

    public void testLineComment5() {
        AstRoot root = parse("  /**\n* a.\n* b.\n* c.*/\n");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals(0, root.getComments().first().getLineno());
    }

    public void testLineComment6() {
        AstRoot root = parse("  \n/**\n* a.\n* b.\n* c.*/\n");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals(1, root.getComments().first().getLineno());
    }

    public void testLinenoLiteral() {
        AstRoot root = parse(
            "\nvar d =\n" +
            "    \"foo\";\n" +
            "var e =\n" +
            "    1;\n" +
            "var f = \n" +
            "    1.2;\n" +
            "var g = \n" +
            "    2e5;\n" +
            "var h = \n" +
            "    'bar';\n");

        VariableDeclaration stmt1 = (VariableDeclaration) root.getFirstChild();
        List<VariableInitializer> vars1 = stmt1.getVariables();
        VariableInitializer firstVar = (VariableInitializer) vars1.get(0);
        Name firstVarName = (Name) firstVar.getTarget();
        AstNode firstVarLiteral = firstVar.getInitializer();

        VariableDeclaration stmt2 = (VariableDeclaration) stmt1.getNext();
        List<VariableInitializer> vars2 = stmt2.getVariables();
        VariableInitializer secondVar = (VariableInitializer) vars2.get(0);
        Name secondVarName = (Name) secondVar.getTarget();
        AstNode secondVarLiteral = secondVar.getInitializer();

        VariableDeclaration stmt3 = (VariableDeclaration) stmt2.getNext();
       List<VariableInitializer> vars3 = stmt3.getVariables();
        VariableInitializer thirdVar = (VariableInitializer) vars3.get(0);
        Name thirdVarName = (Name) thirdVar.getTarget();
        AstNode thirdVarLiteral = thirdVar.getInitializer();

        VariableDeclaration stmt4 = (VariableDeclaration) stmt3.getNext();
        List<VariableInitializer> vars4 = stmt4.getVariables();
        VariableInitializer fourthVar = (VariableInitializer) vars4.get(0);
        Name fourthVarName = (Name) fourthVar.getTarget();
        AstNode fourthVarLiteral = fourthVar.getInitializer();

        VariableDeclaration stmt5 = (VariableDeclaration) stmt4.getNext();
        List<VariableInitializer> vars5 = stmt5.getVariables();
        VariableInitializer fifthVar = (VariableInitializer) vars5.get(0);
        Name fifthVarName = (Name) fifthVar.getTarget();
        AstNode fifthVarLiteral = fifthVar.getInitializer();

        assertEquals(2, firstVarLiteral.getLineno());
        assertEquals(4, secondVarLiteral.getLineno());
        assertEquals(6, thirdVarLiteral.getLineno());
        assertEquals(8, fourthVarLiteral.getLineno());
        assertEquals(10, fifthVarLiteral.getLineno());
    }

    public void testLinenoSwitch() {
        AstRoot root = parse(
            "\nswitch (a) {\n" +
            "   case\n" +
            "     1:\n" +
            "     b++;\n" +
            "   case 2:\n" +
            "   default:\n" +
            "     b--;\n" +
            "  }\n");

        SwitchStatement switchStmt = (SwitchStatement) root.getFirstChild();
        AstNode switchVar = switchStmt.getExpression();
        List<SwitchCase> cases = switchStmt.getCases();
        SwitchCase firstCase = cases.get(0);
        AstNode caseArg = (AstNode) firstCase.getExpression();
        List<AstNode> caseBody = firstCase.getStatements();
        ExpressionStatement exprStmt = (ExpressionStatement) caseBody.get(0);
        UnaryExpression incrExpr = (UnaryExpression) exprStmt.getExpression();
        AstNode incrVar = (AstNode) incrExpr.getOperand();

        SwitchCase secondCase = cases.get(1);
        AstNode defaultCase = cases.get(2);
        AstNode returnStmt = (AstNode) switchStmt.getNext();

        assertEquals(1, switchStmt.getLineno());
        assertEquals(1, switchVar.getLineno());
        assertEquals(2, firstCase.getLineno());
        assertEquals(3, caseArg.getLineno());
        assertEquals(4, exprStmt.getLineno());
        assertEquals(4, incrExpr.getLineno());
        assertEquals(4, incrVar.getLineno());
        assertEquals(5, secondCase.getLineno());
        assertEquals(6, defaultCase.getLineno());
    }


    public void testLinenoFunctionParams() {
        AstRoot root = parse(
            "\nfunction\n" +
            "    foo(\n" +
            "    a,\n" +
            "    b,\n" +
            "    c) {\n" +
            "}\n");
        FunctionNode function = (FunctionNode) root.getFirstChild();
        Name functionName = (Name) function.getFunctionName();

        AstNode body = function.getBody();
        List<AstNode> params = function.getParams();
        AstNode param1 = params.get(0);
        AstNode param2 = params.get(1);
        AstNode param3 = params.get(2);

        assertEquals(1, function.getLineno());
        assertEquals(2, functionName.getLineno());
        assertEquals(3, param1.getLineno());
        assertEquals(4, param2.getLineno());
        assertEquals(5, param3.getLineno());
        assertEquals(5, body.getLineno());
    }

    public void testLinenoVarDecl() {
      AstRoot root = parse(
          "\nvar\n" +
          "    a =\n" +
          "    3\n");

      VariableDeclaration decl = (VariableDeclaration) root.getFirstChild();
      List<VariableInitializer> vars = decl.getVariables();
      VariableInitializer init = (VariableInitializer) vars.get(0);
      AstNode declName = init.getTarget();
      AstNode expr = init.getInitializer();

      assertEquals(1, decl.getLineno());
      assertEquals(2, init.getLineno());
      assertEquals(2, declName.getLineno());
      assertEquals(3, expr.getLineno());
    }

    public void testLinenoReturn() {
      AstRoot root = parse(
          "\nfunction\n" +
          "    foo(\n" +
          "    a,\n" +
          "    b,\n" +
          "    c) {\n" +
          "    return\n" +
          "    4;\n" +
          "}\n");
      FunctionNode function = (FunctionNode) root.getFirstChild();
      Name functionName = (Name) function.getFunctionName();

      AstNode body = function.getBody();
      ReturnStatement returnStmt = (ReturnStatement) body.getFirstChild();
      ExpressionStatement exprStmt = (ExpressionStatement) returnStmt.getNext();
      AstNode returnVal = exprStmt.getExpression();

      assertEquals(6, returnStmt.getLineno());
      assertEquals(7, exprStmt.getLineno());
      assertEquals(7, returnVal.getLineno());
    }

    public void testLinenoFor() {
        AstRoot root = parse(
            "\nfor(\n" +
            ";\n" +
            ";\n" +
            ") {\n" +
            "}\n");

        ForLoop forLoop = (ForLoop) root.getFirstChild();
        AstNode initClause = forLoop.getInitializer();
        AstNode condClause = forLoop.getCondition();
        AstNode incrClause = forLoop.getIncrement();

      assertEquals(1, forLoop.getLineno());
      assertEquals(2, initClause.getLineno());
      assertEquals(3, condClause.getLineno());
      assertEquals(4, incrClause.getLineno());
    }

    public void testLinenoInfix() {
        AstRoot root = parse(
            "\nvar d = a\n" +
            "    + \n" +
            "    b;\n" +
            "var\n" +
            "    e =\n" +
            "    a +\n" +
            "    c;\n" +
            "var f = b\n" +
            "    / c;\n");

        VariableDeclaration stmt1 = (VariableDeclaration) root.getFirstChild();
        List<VariableInitializer> vars1 = stmt1.getVariables();
        VariableInitializer var1 = (VariableInitializer) vars1.get(0);
        Name firstVarName = (Name) var1.getTarget();
        InfixExpression var1Add = (InfixExpression) var1.getInitializer();

        VariableDeclaration stmt2 = (VariableDeclaration) stmt1.getNext();
        List<VariableInitializer> vars2 = stmt2.getVariables();
        VariableInitializer var2 = (VariableInitializer) vars2.get(0);
        Name secondVarName = (Name) var2.getTarget();
        InfixExpression var2Add = (InfixExpression) var2.getInitializer();

        VariableDeclaration stmt3 = (VariableDeclaration) stmt2.getNext();
        List<VariableInitializer> vars3 = stmt3.getVariables();
        VariableInitializer var3 = (VariableInitializer) vars3.get(0);
        Name thirdVarName = (Name) var3.getTarget();
        InfixExpression thirdVarDiv = (InfixExpression) var3.getInitializer();

        ReturnStatement returnStmt = (ReturnStatement) stmt3.getNext();

        assertEquals(1, var1.getLineno());
        assertEquals(1, firstVarName.getLineno());
        assertEquals(2, var1Add.getLineno());
        assertEquals(1, var1Add.getLeft().getLineno());
        assertEquals(3, var1Add.getRight().getLineno());

        // var directive with name on next line wrong --
        // should be 6.
        assertEquals(5, var2.getLineno());
        assertEquals(5, secondVarName.getLineno());
        assertEquals(6, var2Add.getLineno());
        assertEquals(6, var2Add.getLeft().getLineno());
        assertEquals(7, var2Add.getRight().getLineno());

        assertEquals(8, var3.getLineno());
        assertEquals(8, thirdVarName.getLineno());
        assertEquals(9, thirdVarDiv.getLineno());
        assertEquals(8, thirdVarDiv.getLeft().getLineno());
        assertEquals(9, thirdVarDiv.getRight().getLineno());
    }

    public void testLinenoPrefix() {
        AstRoot root = parse(
            "\na++;\n" +
            "   --\n" +
            "   b;\n");

        ExpressionStatement first = (ExpressionStatement) root.getFirstChild();
        ExpressionStatement secondStmt = (ExpressionStatement) first.getNext();
        UnaryExpression firstOp = (UnaryExpression) first.getExpression();
        UnaryExpression secondOp = (UnaryExpression) secondStmt.getExpression();
        AstNode firstVarRef = firstOp.getOperand();
        AstNode secondVarRef = secondOp.getOperand();

        assertEquals(1, firstOp.getLineno());
        assertEquals(2, secondOp.getLineno());
        assertEquals(1, firstVarRef.getLineno());
        assertEquals(3, secondVarRef.getLineno());
      }

    public void testLinenoIf() {
        AstRoot root = parse(
            "\nif\n" +
            "   (a == 3)\n" +
            "   {\n" +
            "     b = 0;\n" +
            "   }\n" +
            "     else\n" +
            "   {\n" +
            "     c = 1;\n" +
            "   }\n");

        IfStatement ifStmt = (IfStatement) root.getFirstChild();
        AstNode condClause = ifStmt.getCondition();
        AstNode thenClause = ifStmt.getThenPart();
        AstNode elseClause = ifStmt.getElsePart();

        assertEquals(1, ifStmt.getLineno());
        assertEquals(2, condClause.getLineno());
        assertEquals(3, thenClause.getLineno());
        assertEquals(7, elseClause.getLineno());
    }


    public void testLinenoTry() {
        AstRoot root = parse(
            "\ntry {\n" +
            "    var x = 1;\n" +
            "} catch\n" +
            "    (err)\n" +
            "{\n" +
            "} finally {\n" +
            "    var y = 2;\n" +
            "}\n");

        TryStatement tryStmt = (TryStatement) root.getFirstChild();
        AstNode tryBlock = tryStmt.getTryBlock();
        List<CatchClause> catchBlocks = tryStmt.getCatchClauses();
        CatchClause catchClause= catchBlocks.get(0);
        Block catchVarBlock = (Block) catchClause.getBody();
        Name catchVar = (Name) catchClause.getVarName();
        AstNode finallyBlock = tryStmt.getFinallyBlock();
        AstNode finallyStmt = (AstNode) finallyBlock.getFirstChild();

        assertEquals(1, tryStmt.getLineno());
        assertEquals(1, tryBlock.getLineno());
        assertEquals(5, catchVarBlock.getLineno());
        assertEquals(4, catchVar.getLineno());
        assertEquals(3, catchClause.getLineno());
        assertEquals(6, finallyBlock.getLineno());
        assertEquals(7, finallyStmt.getLineno());
      }

    public void testLinenoConditional() {
         AstRoot root = parse(
            "\na\n" +
            "    ?\n" +
            "    b\n" +
            "    :\n" +
            "    c\n" +
            "    ;\n");

        ExpressionStatement ex = (ExpressionStatement) root.getFirstChild();
        ConditionalExpression hook = (ConditionalExpression) ex.getExpression();
        AstNode condExpr = hook.getTestExpression();
        AstNode thenExpr = hook.getTrueExpression();
        AstNode elseExpr = hook.getFalseExpression();

        assertEquals(2, hook.getLineno());
        assertEquals(1, condExpr.getLineno());
        assertEquals(3, thenExpr.getLineno());
        assertEquals(5, elseExpr.getLineno());
    }

    public void testLinenoLabel() {
        AstRoot root = parse(
            "\nfoo:\n" +
            "a = 1;\n" +
            "bar:\n" +
            "b = 2;\n");

        LabeledStatement firstStmt = (LabeledStatement) root.getFirstChild();
        LabeledStatement secondStmt = (LabeledStatement) firstStmt.getNext();

        assertEquals(1, firstStmt.getLineno());
        assertEquals(3, secondStmt.getLineno());
    }

    public void testLinenoCompare() {
        AstRoot root = parse(
            "\na\n" +
            "<\n" +
            "b\n");

        ExpressionStatement expr = (ExpressionStatement) root.getFirstChild();
        InfixExpression compare = (InfixExpression) expr.getExpression();
        AstNode lhs = compare.getLeft();
        AstNode rhs = compare.getRight();

        assertEquals(1, lhs.getLineno());
        assertEquals(2, compare.getLineno());
        assertEquals(3, rhs.getLineno());
    }

    public void testLinenoEq() {
        AstRoot root = parse(
            "\na\n" +
            "==\n" +
            "b\n");
        ExpressionStatement expr = (ExpressionStatement) root.getFirstChild();
        InfixExpression compare = (InfixExpression) expr.getExpression();
        AstNode lhs = compare.getLeft();
        AstNode rhs = compare.getRight();

        assertEquals(1, lhs.getLineno());
        assertEquals(2, compare.getLineno());
        assertEquals(3, rhs.getLineno());
    }

    public void testLinenoPlusEq() {
        AstRoot root = parse(
            "\na\n" +
            "+=\n" +
            "b\n");
        ExpressionStatement expr = (ExpressionStatement) root.getFirstChild();
        Assignment assign = (Assignment) expr.getExpression();
        AstNode lhs = assign.getLeft();
        AstNode rhs = assign.getRight();

        assertEquals(1, lhs.getLineno());
        assertEquals(2, assign.getLineno());
        assertEquals(3, rhs.getLineno());
    }

    public void testLinenoComma() {
        AstRoot root = parse(
            "\na,\n" +
            "    b,\n" +
            "    c;\n");

        ExpressionStatement stmt = (ExpressionStatement) root.getFirstChild();
        InfixExpression comma1 = (InfixExpression) stmt.getExpression();
        InfixExpression comma2 = (InfixExpression) comma1.getLeft();
        AstNode cRef = comma1.getRight();
        AstNode aRef = comma2.getLeft();
        AstNode bRef = comma2.getRight();

        assertEquals(2, comma1.getLineno());
        assertEquals(1, comma2.getLineno());
        assertEquals(1, aRef.getLineno());
        assertEquals(2, bRef.getLineno());
        assertEquals(3, cRef.getLineno());
    }

    public void testInOperatorInForLoop1() {
        parse("var a={};function b_(p){ return p;};" +
              "for(var i=b_(\"length\" in a);i<0;) {}");
    }

    public void testInOperatorInForLoop2() {
        parse("var a={}; for (;(\"length\" in a);) {}");
    }

    public void testInOperatorInForLoop3() {
        parse("for (x in y) {}");
    }

    public void testJSDocAttachment1() {
        AstRoot root = parse("/** @type number */var a;");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals("/** @type number */",
                     root.getComments().first().getValue());
        assertNotNull(root.getFirstChild().getJsDoc());
    }

    public void testJSDocAttachment2() {
        AstRoot root = parse("/** @type number */a.b;");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals("/** @type number */",
                     root.getComments().first().getValue());
        ExpressionStatement st = (ExpressionStatement) root.getFirstChild();
        assertNotNull(st.getExpression().getJsDoc());
    }

    public void testJSDocAttachment3() {
        AstRoot root = parse("var a = /** @type number */(x);");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals("/** @type number */",
                     root.getComments().first().getValue());
        VariableDeclaration vd = (VariableDeclaration) root.getFirstChild();
        VariableInitializer vi = vd.getVariables().get(0);
        assertNotNull(vi.getInitializer().getJsDoc());
    }

    public void testJSDocAttachment4() {
        AstRoot root = parse("(function() {/** should not be attached */})()");
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        ExpressionStatement st = (ExpressionStatement) root.getFirstChild();
        FunctionCall fc = (FunctionCall) st.getExpression();
        ParenthesizedExpression pe = (ParenthesizedExpression) fc.getTarget();
        assertNull(pe.getJsDoc());
    }

    public void testParsingWithoutJSDoc() {
        AstRoot root = parse("var a = /** @type number */(x);", false);
        assertNotNull(root.getComments());
        assertEquals(1, root.getComments().size());
        assertEquals("/** @type number */",
                     root.getComments().first().getValue());
        VariableDeclaration vd = (VariableDeclaration) root.getFirstChild();
        VariableInitializer vi = vd.getVariables().get(0);
        assertTrue(vi.getInitializer() instanceof ParenthesizedExpression);
    }

    public void testParseCommentsAsReader() throws IOException {
        AstRoot root = parseAsReader(
            "/** a */var a;\n /** b */var b; /** c */var c;");
        assertNotNull(root.getComments());
        assertEquals(3, root.getComments().size());
        Comment[] comments = new Comment[3];
        comments = root.getComments().toArray(comments);
        assertEquals("/** a */", comments[0].getValue());
        assertEquals("/** b */", comments[1].getValue());
        assertEquals("/** c */", comments[2].getValue());
    }

    public void testParseCommentsAsReader2() throws IOException {
        String js = "";
        for (int i = 0; i < 100; i++) {
            String stri = Integer.toString(i);
            js += "/** Some comment for a" + stri + " */" +
                  "var a" + stri + " = " + stri + ";\n";
        }
        AstRoot root = parseAsReader(js);
    }
    
    private AstRoot parse(String string) {
        return parse(string, true);    
    }

    private AstRoot parse(String string, boolean jsdoc) {
        CompilerEnvirons environment = new CompilerEnvirons();

        TestErrorReporter testErrorReporter = new TestErrorReporter(null, null);
        environment.setErrorReporter(testErrorReporter);

        environment.setRecordingComments(true);
        environment.setRecordingLocalJsDocComments(jsdoc);

        Parser p = new Parser(environment, testErrorReporter);
        AstRoot script = p.parse(string, null, 0);

        assertTrue(testErrorReporter.hasEncounteredAllErrors());
        assertTrue(testErrorReporter.hasEncounteredAllWarnings());

        return script;
    }

    private AstRoot parseAsReader(String string) throws IOException {
        CompilerEnvirons environment = new CompilerEnvirons();

        TestErrorReporter testErrorReporter = new TestErrorReporter(null, null);
        environment.setErrorReporter(testErrorReporter);

        environment.setRecordingComments(true);
        environment.setRecordingLocalJsDocComments(true);

        Parser p = new Parser(environment, testErrorReporter);
        AstRoot script = p.parse(new StringReader(string), null, 0);

        assertTrue(testErrorReporter.hasEncounteredAllErrors());
        assertTrue(testErrorReporter.hasEncounteredAllWarnings());

        return script;
    }
}
