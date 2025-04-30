<?php

namespace IPP\Student\Primitives;

use IPP\Student\InternalStructs\Send;
use IPP\Student\InternalStructs\Program;
use IPP\Student\Interpreter;

class FalseClass extends ObjectClass
{
    private static ?RuntimeObject $falseObj = null;

    public function __construct()
    {
        $this->name = 'False';
        $this->parent = 'Object';

        $this->methodTable = [
            'not' => [FalseClass::class, 'not'],
            'and:'  => [FalseClass::class, 'and'],
            'or:' => [FalseClass::class, 'or'],
            'ifTrue:ifFalse:' => [FalseClass::class, 'ifTrueIfFalse'],
            'asString' => [FalseClass::class, 'asString']
        ];
    }

    public static function getFalse(Program $program): RuntimeObject
    {
        if (FalseClass::$falseObj === null) {
            FalseClass::$falseObj = new RuntimeObject('False', $program);
        }

        return FalseClass::$falseObj;
    }

    /** @param array<RuntimeObject> $args  */
    public static function not(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        if ($thisObject->internal === true) {
            return FalseClass::getFalse($program);
        }

        return TrueClass::getTrue($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function and(
        RuntimeObject $thisObject,
        array $args,
        Program $program,
        Interpreter $interpreter
    ): RuntimeObject {
        if ($thisObject->internal === false) {
            return FalseClass::getFalse($program);
        }

        $message = new Send('value');
        $message->args = [];
        $message->receiver = $args[0];

        return $interpreter->interpretSend($interpreter->stack->topFrame(), $message);
    }

    /** @param array<RuntimeObject> $args  */
    public static function or(
        RuntimeObject $thisObject,
        array $args,
        Program $program,
        Interpreter $interpreter
    ): RuntimeObject {
        if ($thisObject->internal === true) {
            return TrueClass::getTrue($program);
        }

        $message = new Send('value');
        $message->args = [];
        $message->receiver = $args[0];

        return $interpreter->interpretSend($interpreter->stack->topFrame(), $message);
    }

    /** @param array<RuntimeObject> $args  */
    public static function ifTrueIfFalse(
        RuntimeObject $thisObject,
        array $args,
        Program $program,
        Interpreter $interpreter
    ): RuntimeObject {
        $message = new Send('value');
        $message->args = [];

        if ($thisObject->internal === true) {
            $message->receiver = $args[0];
        } else {
            $message->receiver = $args[1];
        }

        return $interpreter->interpretSend($interpreter->stack->topFrame(), $message);
    }

    public static function asString(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        return RuntimeObject::createAndSetInstance('String', 'false', $program);
    }
}
