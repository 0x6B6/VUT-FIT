<?php

namespace IPP\Student\InternalStructs;

class Variable extends Expression
{
    public string $name;

    public function __construct(string $name)
    {
        $this->name = $name;
    }

    public function __toString(): string
    {
        return "var {$this->name}";
    }
}
