<?php

namespace IPP\Student;

/* Call Stack */
class Stack
{
    /** @var array<StackFrame> */
    private array $stack = [];

    // Push frame
    public function pushFrame(StackFrame $frame): void
    {
        $this->stack[] = $frame;
    }

    // Pop frame
    public function popFrame(): StackFrame|null
    {
        return array_pop($this->stack);
    }
    // Top frame
    public function topFrame(): StackFrame|false
    {
        return end($this->stack);
    }

    public function __toString(): string
    {
        $num_frames = count($this->stack);

        return "Number of stack frames: {$num_frames}";
    }
}
