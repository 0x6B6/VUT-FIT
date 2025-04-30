<?php

namespace IPP\Student;

use IPP\Student\Primitives\RuntimeObject;

/* Call Stack Frame for variables and parameters */
class StackFrame
{
    // variable list
    /** @var array<string, RuntimeObject> */
    public array $vars = [];

    // param list
    /** @var array<string, RuntimeObject> */
    public array $params = [];

    public function __toString(): string
    {
        $frameVars = print_r($this->vars, true);
        $frameParams = print_r($this->params, true);

        return "Frame variables:\n{$frameVars}\n\nFrame parameters:\n{$frameParams}";
    }
}
