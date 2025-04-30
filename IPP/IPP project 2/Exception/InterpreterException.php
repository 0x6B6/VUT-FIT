<?php

namespace IPP\Student\Exception;

use IPP\Core\Exception\IPPException;
use Throwable;

/**
 * Exception for runtime interpretation errors
 */
class InterpreterException extends IPPException
{
    public function __construct(string $message, int $code, ?Throwable $previous = null)
    {
        parent::__construct($message, $code, $previous, false);
    }
}
