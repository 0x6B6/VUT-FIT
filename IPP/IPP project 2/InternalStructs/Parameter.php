<?php

namespace IPP\Student\InternalStructs;

class Parameter
{
    public string $name;
    public int $order;

    public function __construct(int $order, string $name)
    {
        $this->order = $order;
        $this->name = $name;
    }

    public function __toString(): string
    {
        return $this->name;
    }
}
