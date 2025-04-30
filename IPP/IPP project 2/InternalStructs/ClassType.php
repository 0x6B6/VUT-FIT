<?php

namespace IPP\Student\InternalStructs;

class ClassType
{
    public string $name;
    public string $parent;
    /** @var array<Method> */
    public array $methodTable = [];

    public function __construct(string $name, string $parent)
    {
        $this->name = $name;
        $this->parent = $parent;
    }

    public function __toString(): string
    {
        $methods = "";

        foreach ($this->methodTable as $method) {
            $methods .= "  {$method}\n";
        }

        return "class {$this->name} : {$this->parent} {\n{$methods}}\n";
    }
}
