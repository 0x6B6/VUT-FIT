<?php

namespace IPP\Student\InternalStructs;

use IPP\Student\Primitives\RuntimeObject;

class Block extends Expression
{
    /** @var array<Parameter> */
    public array $params = [];
    /** @var array<Assignment> */
    public array $statements = [];
    public int $arity;
    public RuntimeObject $self;

    public function __construct(int $arity)
    {
        $this->arity = $arity;
    }

    public function __toString(): string
    {
        $assigns = "";

        foreach ($this->statements as $assign) {
            $assigns .= $assign . "\n";
        }

        $params = "";

        foreach ($this->params as $param) {
            $params .= ":" . $param . " ";
        }

        return "[ {$params} |\n{$assigns}      ]";
    }
}
