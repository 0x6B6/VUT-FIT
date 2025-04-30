<?php

namespace IPP\Student\Primitives;

use IPP\Student\InternalStructs\Send;
use IPP\Student\InternalStructs\Program;
use IPP\Student\Interpreter;

class TrueClass extends ObjectClass
{
    private static ?RuntimeObject $trueObj = null;

    public function __construct()
    {
        $this->name = 'True';
        $this->parent = 'Object';

        $this->methodTable = [
            'not' => [TrueClass::class, 'not'],
            'and:'  => [TrueClass::class, 'and'],
            'or:' => [TrueClass::class, 'or'],
            'ifTrue:ifFalse:' => [TrueClass::class, 'ifTrueIfFalse'],
            'asString' => [TrueClass::class, 'asString']
        ];
    }

    public static function getTrue(Program $program): RuntimeObject
    {
        if (TrueClass::$trueObj === null) {
            TrueClass::$trueObj = new RuntimeObject('True', $program);
        }

        return TrueClass::$trueObj;
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
        return RuntimeObject::createAndSetInstance('String', 'true', $program);
    }
}
