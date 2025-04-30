<?php

namespace IPP\Student\Primitives;

use IPP\Student\InternalStructs\Program;

class ObjectClass
{
    public string $name;
    public ?string $parent;
    /** @var array<string, array{0: class-string, 1: string}> */
    public array $methodTable = [];

    public function __construct()
    {
        $this->name = 'Object';
        $this->parent = null;

        $this->methodTable = [
            'identicalTo:' => [ObjectClass::class, 'identicalTo'],
            'equalTo:'  => [ObjectClass::class, 'equalTo'],
            'asString' => [ObjectClass::class, 'asString'],
            'isNumber' => [ObjectClass::class, 'isNumber'],
            'isString' => [ObjectClass::class, 'isString'],
            'isBlock' => [ObjectClass::class, 'isBlock'],
            'isNil' => [ObjectClass::class, 'isNil']
        ];
    }

    /** @param array<RuntimeObject> $args  */
    public static function identicalTo(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        $otherObject = $args[0];

        if ($thisObject === $otherObject) {
            return TrueClass::getTrue($program);
        }

        return FalseClass::getFalse($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function equalTo(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return ObjectClass::identicalTo($thisObject, $args, $program);
    }

    public static function asString(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        return new RuntimeObject('String', $program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function isNumber(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return FalseClass::getFalse($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function isString(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return FalseClass::getFalse($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function isBlock(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return FalseClass::getFalse($program);
    }

    /** @param array<RuntimeObject> $args  */
    public static function isNil(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return FalseClass::getFalse($program);
    }

    public function __toString(): string
    {
        $parent = $this->parent ?? 'null';
        return "class {$this->name} : {$parent} {}\n";
    }
}
