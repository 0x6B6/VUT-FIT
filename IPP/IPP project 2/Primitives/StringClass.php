<?php

namespace IPP\Student\Primitives;

use IPP\Core\Interface\InputReader;
use IPP\Core\Interface\OutputWriter;
use IPP\Core\ReturnCode;
use IPP\Student\Exception\InterpreterException;
use IPP\Student\InternalStructs\Program;

class StringClass extends ObjectClass
{
    public function __construct()
    {
        $this->name = 'String';
        $this->parent = 'Object';

        $this->methodTable = [
            'isString' => [StringClass::class, 'isString'],
            'read' => [StringClass::class, 'read'],
            'print'  => [StringClass::class, 'print'],
            'equalTo:' => [StringClass::class, 'equalTo'],
            'asString' => [StringClass::class, 'asString'],
            'asInteger' => [StringClass::class, 'asInteger'],
            'concatenateWith:' => [StringClass::class, 'concatenateWith'],
            'startsWith:endsBefore:' => [StringClass::class, 'startsWithEndsBefore']
        ];
    }

    /** @param array<RuntimeObject> $args  */
    public static function isString(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return TrueClass::getTrue($program);
    }

    public static function read(InputReader $input, Program $program): RuntimeObject
    {
        $strInput = $input->readString();

        if ($strInput === null) {
            throw new InterpreterException("Invalid input - null value", ReturnCode::INPUT_FILE_ERROR);
        }

        return RuntimeObject::createAndSetInstance('String', $strInput, $program);
    }

    public static function print(RuntimeObject $thisObject, RuntimeObject $self, OutputWriter $stdout): RuntimeObject
    {
        $stdout->writeString((string) stripcslashes((string) $thisObject->internal));
        return $self;
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

    public static function asString(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        return $thisObject;
    }

    public static function asInteger(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        $value = 0;

        if (filter_var($thisObject->internal, FILTER_VALIDATE_INT) !== false) {
            $value =  intval($thisObject->internal);
        } else {
            return NilClass::getNil($program);
        }

        return RuntimeObject::createAndSetInstance('Integer', $value, $program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function concatenateWith(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];

        if ($otherObject->primitiveType !== 'String') {
            return NilClass::getNil($program);
        }

        $str = $thisObject->internal . $otherObject->internal;

        return RuntimeObject::createAndSetInstance('String', $str, $program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function startsWithEndsBefore(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        IntegerClass::checkArgType($args[0]->primitiveType);
        IntegerClass::checkArgType($args[1]->primitiveType);

        $start = $args[0]->internal;
        $endBefore = $args[1]->internal;

        if ($start < 1 || $endBefore < 1) {
            return NilClass::getNil($program);
        }

        if (($endBefore - $start) <= 0) {
            return new RuntimeObject('String', $program);
        }

        return RuntimeObject::createAndSetInstance(
            'String',
            substr($thisObject->internal, $start - 1, ($endBefore - $start)),
            $program
        );
    }
}
