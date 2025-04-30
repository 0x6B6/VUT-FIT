<?php

namespace IPP\Student\Primitives;

use IPP\Core\ReturnCode;
use IPP\Student\Exception\InterpreterException;
use IPP\Student\InternalStructs\Program;
use IPP\Student\InternalStructs\Send;
use IPP\Student\Interpreter;

class IntegerClass extends ObjectClass
{
    public function __construct()
    {
        $this->name = 'Integer';
        $this->parent = 'Object';

        $this->methodTable = [
            'isNumber' => [IntegerClass::class, 'isNumber'],
            'equalTo:' => [IntegerClass::class, 'equalTo'],
            'greaterThan:'  => [IntegerClass::class, 'greaterThan'],
            'plus:' => [IntegerClass::class, 'plus'],
            'minus:'  => [IntegerClass::class, 'minus'],
            'multiplyBy:' => [IntegerClass::class, 'multiplyBy'],
            'divBy:' => [IntegerClass::class, 'divBy'],
            'asString' => [IntegerClass::class, 'asString'],
            'asInteger' => [IntegerClass::class, 'asInteger'],
            'timesRepeat:' => [IntegerClass::class, 'timesRepeat']
        ];
    }

    public static function checkArgType(string $argPrimitiveType): void
    {
        if ($argPrimitiveType !== 'Integer') {
            throw new InterpreterException("Argument not Integer type", ReturnCode::INTERPRET_VALUE_ERROR);
        }
    }

    /** @param array<RuntimeObject> $args  */
    public static function isNumber(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return TrueClass::getTrue($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function equalTo(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];

        if (
            $thisObject->primitiveType === $otherObject->primitiveType
            && $thisObject->internal === $otherObject->internal
        ) {
            return TrueClass::getTrue($program);
        }

        return FalseClass::getFalse($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function greaterThan(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];
        IntegerClass::checkArgType($otherObject->primitiveType);

        if ($thisObject->internal > $otherObject->internal) {
            return TrueClass::getTrue($program);
        }

        return FalseClass::getFalse($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function plus(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];
        IntegerClass::checkArgType($otherObject->primitiveType);
        $sum = $thisObject->internal + $otherObject->internal;

        return RuntimeObject::createAndSetInstance('Integer', $sum, $program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function minus(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];
        IntegerClass::checkArgType($otherObject->primitiveType);
        $sum = $thisObject->internal - $otherObject->internal;

        return RuntimeObject::createAndSetInstance('Integer', $sum, $program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function multiplyBy(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];
        IntegerClass::checkArgType($otherObject->primitiveType);
        $sum = $thisObject->internal * $otherObject->internal;

        return RuntimeObject::createAndSetInstance('Integer', $sum, $program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function divBy(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];
        IntegerClass::checkArgType($otherObject->primitiveType);

        if ($otherObject->internal === 0) {
            throw new InterpreterException("Attempted division with zero", ReturnCode::INTERPRET_VALUE_ERROR);
        }

        $sum = $thisObject->internal / $otherObject->internal;

        return RuntimeObject::createAndSetInstance('Integer', $sum, $program);
    }

    public static function asString(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        return RuntimeObject::createAndSetInstance('String', strval($thisObject->internal), $program);
    }

    public static function asInteger(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        return $thisObject;
    }

    /** @param array<RuntimeObject> $args  */
    public static function timesRepeat(
        RuntimeObject $thisObject,
        array $args,
        Program $program,
        Interpreter $interpreter
    ): RuntimeObject {
        $otherObject = $args[0];

        $message = new Send('value:');
        $message->receiver = $otherObject;

        $result = NilClass::getNil($program);

        for ($i = 1; $i <= $thisObject->internal; $i++) {
            $iteration = RuntimeObject::createAndSetInstance('Integer', $i, $program);
            $message->args = [$iteration];

            $result = $interpreter->interpretSend($interpreter->stack->topFrame(), $message);
        }

        return $result;
    }
}
