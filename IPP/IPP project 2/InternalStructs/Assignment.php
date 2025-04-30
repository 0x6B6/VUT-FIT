<?php

namespace IPP\Student\InternalStructs;

class Assignment
{
    public string $targetVarName;
    public int $order;
    public Expression $expr;

    public function __construct(int $order)
    {
        $this->order = $order;
    }

    public function __toString(): string
    {
        return "    {$this->targetVarName} := {$this->expr}.";
    }
}
