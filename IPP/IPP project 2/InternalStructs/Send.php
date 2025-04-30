<?php

namespace IPP\Student\InternalStructs;

use IPP\Student\Primitives\RuntimeObject;

class Send extends Expression
{
    public string $selector;
    /** @var array<int, Expression|RuntimeObject>  */
    public array $args = [];
    public Expression|RuntimeObject $receiver;

    public function __construct(string $selector)
    {
        $this->selector = $selector;
    }

    public function __toString(): string
    {
        $args = "";

        foreach ($this->args as $arg) {
            $args .= "{$arg}, ";
        }

        return "{$this->receiver} {$this->selector}({$args})";
    }
}
