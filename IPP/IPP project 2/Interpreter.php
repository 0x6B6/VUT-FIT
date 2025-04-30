<?php

namespace IPP\Student;

use IPP\Core\AbstractInterpreter;
use IPP\Core\ReturnCode;
use IPP\Student\Exception\InterpreterException;
use IPP\Student\InternalStructs\Program;
use IPP\Student\InternalStructs\Block;
use IPP\Student\InternalStructs\Method;
use IPP\Student\InternalStructs\Assignment;
use IPP\Student\InternalStructs\Expression;
use IPP\Student\InternalStructs\Variable;
use IPP\Student\InternalStructs\Literal;
use IPP\Student\InternalStructs\Send;
use IPP\Student\Primitives\ObjectClass;
use IPP\Student\Primitives\NilClass;
use IPP\Student\Primitives\IntegerClass;
use IPP\Student\Primitives\StringClass;
use IPP\Student\Primitives\BlockClass;
use IPP\Student\Primitives\FalseClass;
use IPP\Student\Primitives\TrueClass;
use IPP\Student\Primitives\RuntimeObject;

/* Interpreter core */
class Interpreter extends AbstractInterpreter
{
    /* Internal representation of SOL25 program */
    private Program $program;

    /* Internal interpretation memory */
    public Stack $stack;
    public RuntimeObject $self;
    /** @var array<RuntimeObject> */
    private array $selfStack = [];

    /* Creates a runtime instance from argument instance */
    private function createFrom(Literal $literal, RuntimeObject $otherObject): RuntimeObject
    {
        $class = $literal->value;
        $newObject = new RuntimeObject($class, $this->program);

        // If the other objects primitive type is 'Object' return new object with default value,
        // since Object class is the SuperClass of all available classes.
        if ($otherObject->primitiveType === 'Object') {
            return $newObject;
        }

        // Reference object's class is not a subclass/superclass or same class as literal
        if ($newObject->primitiveType !== $otherObject->primitiveType) {
            throw new InterpreterException(
                "Class method 'from: error' - given object's class/superclass/subclass not matching '$class'",
                ReturnCode::INTERPRET_VALUE_ERROR
            );
        }

        $newObject->setInternalAttribute($otherObject->internal);
        $newObject->attributes = [...$otherObject->attributes];

        return $newObject;
    }

    /* Creates a runtime instance from class literal using new constructor */
    private function createNew(Literal $literal): RuntimeObject
    {
        if ($literal->value === 'Nil') {
            return NilClass::getNil($this->program);
        }

        if ($literal->value === 'True') {
            return TrueClass::getTrue($this->program);
        }

        if ($literal->value === 'False') {
            return FalseClass::getFalse($this->program);
        }

        return new RuntimeObject($literal->value, $this->program);
    }

    /** Creates a runtime instance from a Class Literal using Class methods
     *
     * @param Literal $literal
     * @param string $selector
     * @param array<int, RuntimeObject> $args
     * @return RuntimeObject
     */
    private function createClassInstance(Literal $literal, string $selector, array $args): RuntimeObject
    {
        if ($selector === 'new') {
            return $this->createNew($literal);
        }

        if ($selector === 'from:') {
            return $this->createFrom($literal, $args[0]);
        }

        // String Class 'read' method
        if ($selector === 'read' && $literal->value === 'String') {
            return StringClass::read($this->input, $this->program);
        }

        throw new InterpreterException(
            "Class method '$selector' missing, class DNU the message",
            ReturnCode::INTERPRET_DNU_ERROR
        );
    }

    /**
     * Creates a runtime instance from a Literal.
     *
     * @param Literal $literal
     * @return RuntimeObject
     * @throws InterpreterException
     */
    private function createLiteralInstance(Literal $literal): RuntimeObject
    {
        if ($literal->class === 'class') {
            return new RuntimeObject($literal->value, $this->program);
        }

        if ($literal->class === 'String') {
            return RuntimeObject::createAndSetInstance('String', $literal->value, $this->program);
        }

        if ($literal->class === 'Integer') {
            if (filter_var($literal->value, FILTER_VALIDATE_INT) === false) {
                throw new InterpreterException(
                    "Literal value '$literal->value' is not of type Integer",
                    ReturnCode::INTERPRET_TYPE_ERROR
                );
            }

            return RuntimeObject::createAndSetInstance('Integer', intval($literal->value), $this->program);
        }

        if ($literal->class === 'Nil') {
            if ($literal->value !== 'nil') {
                throw new InterpreterException(
                    "Literal value '$literal->value' is not of type Nil",
                    ReturnCode::INTERPRET_TYPE_ERROR
                );
            }
            return NilClass::getNil($this->program);
        }

        if ($literal->class === 'True') {
            if ($literal->value !== 'true') {
                throw new InterpreterException(
                    "Literal value '$literal->value' is not of type True",
                    ReturnCode::INTERPRET_TYPE_ERROR
                );
            }
            return TrueClass::getTrue($this->program);
        }

        if ($literal->class === 'False') {
            if ($literal->value !== 'false') {
                throw new InterpreterException(
                    "Literal value '$literal->value' is not of type False",
                    ReturnCode::INTERPRET_TYPE_ERROR
                );
            }
            return FalseClass::getFalse($this->program);
        }

        throw new InterpreterException(
            "Unknown literal class $literal->class",
            ReturnCode::INTERPRET_TYPE_ERROR
        );
    }

    /* Resolve the whole expression tree */
    private function resolveExpr(StackFrame $frame, Expression $expr): RuntimeObject
    {
        if ($expr instanceof Literal) {
            return $this->createLiteralInstance($expr);
        } elseif ($expr instanceof Send) {
            return $this->interpretSend($frame, $expr);
        } elseif ($expr instanceof Block) {
            $expr->self = $this->self; // Bind current instance self to block
            return RuntimeObject::createAndSetInstance('Block', $expr, $this->program);
        } elseif ($expr instanceof Variable) {
            $var = $expr->name;

            if ($var === 'self' || $var === 'super') { // Self reference
                return $this->self;
            } else {
                if (isset($frame->vars[$var])) { // Variable on stackFrame
                    return $frame->vars[$var];
                } elseif (isset($frame->params[$var])) { // Parameter on stackFrame
                    return $frame->params[$var];
                }
            }
        }

        // Unknown variable/parameter or expression
        throw new InterpreterException("Variable/Parameter/Expression unknown", ReturnCode::INTERPRET_TYPE_ERROR);
    }

    /* Interpret the send message */
    public function interpretSend(StackFrame $frame, Send $send): RuntimeObject
    {
        $super = false;
        $selector = $send->selector;

        // Resolve argument expression(s)
        $args = [];

        foreach ($send->args as $arg) {
            $args[] = $arg instanceof Expression ? $this->resolveExpr($frame, $arg) : $arg;
        }

        // Receiver is a class literal
        if ($send->receiver instanceof Literal && $send->receiver->class === 'class') {
            return $this->createClassInstance($send->receiver, $selector, $args);
        }

        // Receiver is a literal or a variable from an expression
        if ($send->receiver instanceof Expression) {
            $receiver = $this->resolveExpr($frame, $send->receiver);
            $super = $send->receiver instanceof Variable && $send->receiver->name === 'super';
        }

        // Receiver is a runtime object
        if ($send->receiver instanceof RuntimeObject) {
            $receiver = $send->receiver;
        }

        // Methods
        $method = $receiver->methodLookup($selector, $super);

        if ($method !== null) {
            return $this->invokeMethod($receiver, $selector, $method, $args);
        }

        // Block value type messages
        if ($receiver->primitiveType === 'Block') {
            $blockMessage = "value";

            if ($receiver->internal->arity > 0) {
                $blockMessage = str_repeat("value:", $receiver->internal->arity);
            }

            if ($blockMessage === $selector) {
                return $this->interpretBlock($receiver->internal, $args);
            }
        }

        // User Defined Instance Attributes
        $colonCount = substr_count($selector, ':');

        if ($colonCount === 0) { // Attribute access
            if (isset($receiver->attributes[$selector])) {
                return $receiver->attributes[$selector];
            }
        } elseif ($colonCount === 1) { // Attribute initialisation
            $receiver->attributes[rtrim($selector, ':')] = $args[0];
            return $receiver;
        }

        throw new InterpreterException(
            "Method '$selector' missing, object DNU the message",
            ReturnCode::INTERPRET_DNU_ERROR
        );
    }

    /**
     * Invokes the method by its type, which is either an user defined method or a callable
     * builtin class method.
     *
     * Sets the self pseudo variable to the reference of the message receiver object,
     * which is set back to the previous object after the method interpretation  is completed.
     *
     * @param RuntimeObject $receiver
     * @param string $selector
     * @param Method|callable $method
     * @param array<int, RuntimeObject> $args
     * @return RuntimeObject|null
     */
    private function invokeMethod(
        RuntimeObject $receiver,
        string $selector,
        Method|callable $method,
        array $args
    ): RuntimeObject|null {
        /* Store current self reference and get new from the message receiver */
        array_push($this->selfStack, $this->self);
        $this->self = $receiver;

        $result = null;

        /* User defined methods */
        if ($method instanceof Method) {
            $method->block->self = $this->self;
            $result = $this->interpretBlock($method->block, $args);
        }

        /* Builtin methods */
        if (is_callable($method)) {
            switch ($selector) {
                case 'print':
                    $result = $method($receiver, $this->self, $this->stdout);
                    break;

                case 'asString':
                case 'asInteger':
                    $result = $method($receiver, $this->program);
                    break;

                case 'and:':
                case 'or:':
                case 'ifTrue:ifFalse:':
                case 'whileTrue:':
                case 'timesRepeat:':
                    $result = $method($receiver, $args, $this->program, $this);
                    break;

                default:
                    $result = $method($receiver, $args, $this->program);
                    break;
            }
        }

        /* Reference to the previous object self */
        $this->self = array_pop($this->selfStack);

        return $result;
    }

    /**
     * Interprets current statement and puts the result of an expression
     * to the corresponding variable on the current stackframe.
     *
     * @param Assignment $stat
     * @return RuntimeObject
     */
    private function interpretStatement(Assignment $stat): RuntimeObject
    {
        // Current call stack frame
        $frame = $this->stack->topFrame();

        // The identifier of the variable to which the result
        // of the expression is assigned after the command is interpreted.
        $targetVar = $stat->targetVarName;

        $resultObj = $this->resolveExpr($frame, $stat->expr);
        $frame->vars[$targetVar] = $resultObj;

        return $resultObj;
    }

    /**
     * Interprets given Block, that can either be an instance of the block class
     * or a method's body.
     *
     * @param Block $block Method's body or an instance of the Block class
     * @param array<int, RuntimeObject> $args Arguments to be mapped to the block's parameters
     * @return RuntimeObject
     */
    private function interpretBlock(Block $block, array $args): RuntimeObject
    {
        $frame = new StackFrame(); // Create new call stack frame
        $frame->params = array_combine($block->params, $args); // Map arguments to parameter identifiers
        $this->stack->pushFrame($frame);

        array_push($this->selfStack, $this->self);
        $this->self = $block->self;


        $result = NilClass::getNil($this->program); // Store the result of the last statement

        foreach ($block->statements as $statement) {
            $result = $this->interpretStatement($statement);
        }

        $this->stack->popFrame();
        $this->self = array_pop($this->selfStack);

        return $result;
    }

    /**
     * Adds builtin class types to the internal program representation structure
     *
     * @param Program $program
     */
    private function addBuiltinClasses(Program $program): void
    {
        /* Class object list */
        $program->classes['Object'] = new ObjectClass();
        $program->classes['Nil'] = new NilClass();
        $program->classes['Integer'] = new IntegerClass();
        $program->classes['String'] = new StringClass();
        $program->classes['Block'] = new BlockClass();
        $program->classes['True'] = new TrueClass();
        $program->classes['False'] = new FalseClass();

        /* List of built in class types */
        $program->builtinTypes = [
            'Object' => true,
            'Nil' => true,
            'Integer' => true,
            'String' => true,
            'Block' => true,
            'True' => true,
            'False' => true
        ];
    }

    public function execute(): int
    {
        /* Initial interpret setup */
        $dom = $this->source->getDOMDocument();
        $parser = new Parser();
        $this->program = $parser->parse($dom);
        $this->addBuiltinClasses($this->program);
        $this->stack = new Stack();

        /* Check the existence of the Main class */
        if (!array_key_exists('Main', $this->program->classes)) {
            throw new InterpreterException('Missing class Main', ReturnCode::INTERPRET_TYPE_ERROR, null);
        }

        $main = $this->program->classes['Main'];

        /* Check the existence of the run method of the Main class */
        if (!array_key_exists('run', $main->methodTable)) {
            throw new InterpreterException('Missing method run', ReturnCode::INTERPRET_TYPE_ERROR, null);
        }

        /* Interpretation entry point - create an instance of class Main and start to interpret */
        $mainObj = new RuntimeObject('Main', $this->program);
        $this->self = $mainObj;

        $f = $mainObj->methodLookup('run', false);
        $f->block->self = $this->self;
        $this->invokeMethod($mainObj, 'run', $f, []);

        return 0;
    }
}
