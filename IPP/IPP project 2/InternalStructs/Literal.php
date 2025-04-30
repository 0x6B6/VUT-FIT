<?php

namespace IPP\Student\InternalStructs;

class Literal extends Expression
{
    public string $class;
    public string $value;

    public function __construct(string $class, string $value)
    {
        $this->class = $class;
        $this->value = $value;
    }

    public function __toString(): string
    {
        return "{$this->class} literal {$this->value}";
    }
}
