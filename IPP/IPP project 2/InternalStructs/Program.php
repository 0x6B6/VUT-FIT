<?php

namespace IPP\Student\InternalStructs;

use IPP\Student\Primitives\ObjectClass;

class Program
{
    public string $language;
    public string $description;
    /** @var array<ClassType, ObjectClass> */
    public array $classes = [];
    /** @var array<string, bool> */
    public array $builtinTypes;

    public function __construct(string $language, string $description)
    {
        $this->language = $language;
        $this->description = $description;
    }

    public function __toString(): string
    {
        $lang = $this->language;
        $desc = $this->description == "" ? 'None' : $this->description;
        $classQuantity = count($this->classes);
        $classes = "";

        if ($classQuantity > 0) {
            $classes .= ":\n";

            foreach ($this->classes as $class) {
                $classes .= " {$class}";
            }
        } else {
            $classes = " = None";
        }

        return "{$lang} program\nDescription = {$desc}\nClass quantity = {$classQuantity}\nClasses{$classes}\n";
    }
}
