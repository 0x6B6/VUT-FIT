<?php

namespace IPP\Student\InternalStructs;

class Method
{
    public string $selector;
    public Block $block;

    public function __construct(string $selector, Block $block)
    {
        $this->selector = $selector;
        $this->block = $block;
    }

    public function __toString(): string
    {
        return "{$this->selector} {$this->block}\n";
    }
}
