<?php

namespace IPP\Student\Primitives;

use IPP\Student\InternalStructs\Program;
use IPP\Student\InternalStructs\Send;
use IPP\Student\Interpreter;

class BlockClass extends ObjectClass
{
    public function __construct()
    {
        $this->name = 'Block';
        $this->parent = 'Object';

        $this->methodTable = [
            'isBlock' => [BlockClass::class, 'isBlock'],
            'whileTrue:' => [BlockClass::class, 'whileTrue']
        ];
    }

    /** @param array<RuntimeObject> $args  */
    public static function isBlock(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return TrueClass::getTrue($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function whileTrue(
        RuntimeObject $thisObject,
        array $args,
        Program $program,
        Interpreter $interpreter
    ): RuntimeObject {
        $otherObject = $args[0];

        $message = new Send('value');
        $message->args = [];

        $result = NilClass::getNil($program);

        while (true) {
            $message->receiver = $thisObject;
            $receiverResult = $interpreter->interpretSend($interpreter->stack->topFrame(), $message);

            if ($receiverResult->primitiveType !== 'True') {
                break;
            }

            $message->receiver = $otherObject;
            $result = $interpreter->interpretSend($interpreter->stack->topFrame(), $message);
        }

        return $result;
    }
}
