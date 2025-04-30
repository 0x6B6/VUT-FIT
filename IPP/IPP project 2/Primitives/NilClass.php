<?php

namespace IPP\Student\Primitives;

use IPP\Student\InternalStructs\Program;

class NilClass extends ObjectClass
{
    private static ?RuntimeObject $nilObj = null;

    public function __construct()
    {
        $this->name = 'Nil';
        $this->parent = 'Object';

        $this->methodTable = [
            'asString' => [NilClass::class, 'asString'],
            'isNil' => [NilClass::class, 'isNil']
        ];
    }

    public static function getNil(Program $program): RuntimeObject
    {
        if (NilClass::$nilObj === null) {
            NilClass::$nilObj = new RuntimeObject('Nil', $program);
        }

        return NilClass::$nilObj;
    }

    /** @param array<RuntimeObject> $args   */
    public static function isNil(RuntimeObject $thisObject, array $args, Program $program): RuntimeObject
    {
        return TrueClass::getTrue($program);
    }

    public static function asString(RuntimeObject $thisObject, Program $program): RuntimeObject
    {
        return RuntimeObject::createAndSetInstance('String', 'nil', $program);
    }
}
