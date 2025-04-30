<?php

namespace IPP\Student;

use IPP\Student\InternalStructs\Program;
use IPP\Student\InternalStructs\ClassType;
use IPP\Student\InternalStructs\Block;
use IPP\Student\InternalStructs\Parameter;
use IPP\Student\InternalStructs\Method;
use IPP\Student\InternalStructs\Assignment;
use IPP\Student\InternalStructs\Expression;
use IPP\Student\InternalStructs\Variable;
use IPP\Student\InternalStructs\Literal;
use IPP\Student\InternalStructs\Send;
use DOMDocument;
use DOMElement;
use DOMNode;
use IPP\Core\ReturnCode;
use IPP\Student\Exception\InterpreterException;

class Parser
{
    public function parse(DOMDocument $dom): Program
    {
        $progNode = $dom->getElementsByTagName("program")->item(0);

        $lang = Parser::getAttribute($progNode, "language");
        $desc = Parser::getAttribute($progNode, "description");

        $program = new Program($lang, $desc);

        foreach ($progNode->childNodes as $classNode) {
            if (Parser::isNotXmlNode($classNode)) {
                continue;
            }

            $classObj = $this->parseClass($classNode);

            $program->classes[$classObj->name] = $classObj;
        }

        return $program;
    }

    private function parseClass(DOMNode $classNode): ClassType
    {
        $name = Parser::getAttribute($classNode, "name");
        $parent = Parser::getAttribute($classNode, "parent");

        $classObj = new ClassType($name, $parent);

        foreach ($classNode->childNodes as $methodNode) {
            if (Parser::isNotXmlNode($methodNode)) {
                continue;
            }

            $method = $this->parseMethod($methodNode);

            $classObj->methodTable[$method->selector] = $method;
        }

        return $classObj;
    }

    private function parseMethod(DOMNode $method): Method
    {
        $selector = Parser::getAttribute($method, "selector");

        foreach ($method->childNodes as $blockNode) {
            if (Parser::isNotXmlNode($blockNode)) {
                continue;
            }

            $block = $this->parseBlock($blockNode);
            $method = new Method($selector, $block);
            break;
        }

        return $method;
    }

    private function parseBlock(DOMNode $blockNode): Block
    {
        $arity = Parser::getAttribute($blockNode, "arity");

        $block = new Block((int) $arity);

        foreach ($blockNode->childNodes as $Node) {
            if (Parser::isNotXmlNode($Node)) {
                continue;
            }

            if ($Node->nodeName === "assign") {
                $statement = $this->parseAssignment($Node);
                $block->statements[$statement->order] = $statement;
            } elseif ($Node->nodeName == "parameter") {
                $param = $this->parseParameter($Node);
                $block->params[$param->order] = $param;
            }
        }

        // Sort the statements and params by order
        ksort($block->statements);
        ksort($block->params);

        return $block;
    }

    private function parseParameter(DOMNode $paramNode): Parameter
    {
        $order = (int) Parser::getAttribute($paramNode, "order");
        $name = Parser::getAttribute($paramNode, "name");

        return new Parameter($order, $name);
    }

    private function parseAssignment(DOMNode $assignNode): Assignment
    {
        $order = (int) Parser::getAttribute($assignNode, "order");
        $assignment = new Assignment($order);

        foreach ($assignNode->childNodes as $Node) {
            if (Parser::isNotXmlNode($Node)) {
                continue;
            }

            if ($Node->nodeName === "var") {
                $assignment->targetVarName = Parser::getAttribute($Node, "name");
            } elseif ($Node->nodeName === "expr") {
                $assignment->expr = $this->parseExpression($Node);
            }
        }

        return $assignment;
    }

    private function parseExpression(DOMNode $exprNode): Expression
    {
        $expression = null;

        foreach ($exprNode->childNodes as $expr) {
            if (Parser::isNotXmlNode($expr)) {
                continue;
            }

            if ($expr->nodeName == "var") {
                $expression = $this->parseVariable($expr);
            } elseif ($expr->nodeName == "literal") {
                $expression = $this->parseLiteral($expr);
            } elseif ($expr->nodeName == "send") {
                $expression = $this->parseSend($expr);
            } elseif ($expr->nodeName == "block") {
                $expression = $this->parseBlock($expr);
            }

            break;
        }

        return $expression;
    }

    private function parseVariable(DOMNode $varNode): Variable
    {
        $name = Parser::getAttribute($varNode, "name");

        return new Variable($name);
    }

    private function parseLiteral(DOMNode $literalNode): Literal
    {
        $class = Parser::getAttribute($literalNode, "class");
        $value = Parser::getAttribute($literalNode, "value");

        return new Literal($class, $value);
    }

    private function parseSend(DOMNode $sendNode): Send
    {
        $selector = Parser::getAttribute($sendNode, "selector");

        $send = new Send($selector);

        foreach ($sendNode->childNodes as $Node) {
            if (Parser::isNotXmlNode($Node)) {
                continue;
            }

            if ($Node->nodeName === "expr") {
                $send->receiver = $this->parseExpression($Node);
            } elseif ($Node->nodeName === "arg") {
                $order = (int) Parser::getAttribute($Node, "order");

                $send->args[$order] = $this->parseArg($Node);
            }
        }

        // Sort arguments by order
        ksort($send->args);

        return $send;
    }

    private function parseArg(DOMNode $argNode): Expression
    {
        $expr = null;

        foreach ($argNode->childNodes as $arg) {
            if (Parser::isNotXmlNode($arg)) {
                continue;
            }

            if ($arg->nodeName === "expr") {
                $expr = $this->parseExpression($arg);
            }
        }

        return $expr;
    }

    private static function isNotXmlNode(DOMNode $Node): bool
    {
        return $Node->nodeType !== XML_ELEMENT_NODE;
    }

    private static function getAttribute(DOMNode $node, string $attribute): string
    {
        /** @var DOMElement $node */
        return $node->getAttribute($attribute);
    }
}
