<?php

namespace IPP\Student\Primitives;

use IPP\Core\ReturnCode;
use IPP\Student\Exception\InterpreterException;
use IPP\Student\InternalStructs\Program;
use IPP\Student\InternalStructs\Block;
use IPP\Student\InternalStructs\ClassType;
use IPP\Student\InternalStructs\Method;

class RuntimeObject
{
    /* Runtime Object Metadata */
    private Program $program;
    public string $class;
    /** @var array<RuntimeObject> */
    public array $attributes = [];
    public ?string $primitiveType;
    public mixed $internal;

    public function __construct(string $class, Program $program)
    {
        $this->class = $class;
        $this->program = $program;
        $this->getPrimitiveType();
        $this->initInternalAttribute();
    }

    /**
     * Finds the class in the class list by the given key and returns it.
     *
     * @param string $class Class name key
     * @return ClassType|ObjectClass|null User defined class, builtin primitive class or null
     */
    public function getClass(string $class): ClassType|ObjectClass|null
    {
        if (!isset($this->program->classes[$class])) {
            return null;
        }

        return $this->program->classes[$class];
    }

    /**
     * Finds the Super/Parent class in the class list by the given key and returns it.
     *
     * @param string $class Class name key
     * @return ClassType|ObjectClass|null User defined class, builtin primitive class or null
     * @throws InterpreterException When the child class is null (doesnt exist)
     */
    public function getSuper(string $class): ClassType|ObjectClass|null
    {
        $cls = $this->getClass($class);

        if ($cls === null) {
            throw new InterpreterException(
                "Missing class '$class'",
                ReturnCode::INTERPRET_TYPE_ERROR
            );
        }

        if ($cls->parent === null) {
            return null;
        }

        return $this->getClass($cls->parent);
    }

    /**
     * Finds the inherited builtin class of this instance class.
     *
     * @throws InterpreterException when the class of this instance doesn't inherit any builtin class
     *  OR circular inheritance is present
     */
    private function getPrimitiveType(): void
    {
        $class = $this->getClass($this->class);
        $visitedClasses = [];

        while ($class !== null) {
            if (isset($visitedClasses[$class->name])) {
                throw new InterpreterException(
                    "Circular inheritance of class $class",
                    ReturnCode::INTERPRET_TYPE_ERROR
                );
            }

            $visitedClasses[$class->name] = true;

            if (isset($this->program->builtinTypes[$class->name])) {
                $this->primitiveType = $class->name;
                return;
            }

            $class = $this->getSuper($class->name);
        }

        throw new InterpreterException(
            "Missing builtin class inheritance for class $class",
            ReturnCode::INTERPRET_TYPE_ERROR
        );
    }

    /**
     * Initialises the instances internal attribute with corresponding data type and value
     */
    private function initInternalAttribute(): void
    {
        switch ($this->primitiveType) {
            case 'Object':
                $this->setInternalAttribute('None');
                break;

            case 'Nil':
                $this->setInternalAttribute(null);
                break;

            case 'Integer':
                $this->setInternalAttribute(0);
                break;

            case 'String':
                $this->setInternalAttribute('');
                break;

            case 'True':
                $this->setInternalAttribute(true);
                break;

            case 'False':
                $this->setInternalAttribute(false);
                break;

            case 'Block':
                $this->setInternalAttribute(new Block(0));
                break;
        }
    }

    /**
     * Method lookup in the instance classes method table
     *
     *  @param string $selector Message sent to this instance
     *  @return Method|callable Found method
     *  @throws InterpreterException When the instance Does Not Understand the message
     */
    public function methodLookup(string $selector, bool $super): Method|callable|null
    {
        $class = $super ? $this->getSuper($this->class) : $this->getClass($this->class);

        while ($class !== null) {
            if (isset($class->methodTable[$selector])) {
                return $class->methodTable[$selector];
            }

            $class = $this->getSuper($class->name);
        }

        return null;
    }

    /**
     * Creates a new runtime instance of given class and initializes
     * its internal attribute with given value.
     *
     * @param string $class Class to be instantiated
     * @param mixed $value Value to set the instances internal attribute
     * @param Program $program Internal program representation structure
     * @return RuntimeObject New and initialized instance
     */
    public static function createAndSetInstance(string $class, mixed $value, Program $program): RuntimeObject
    {
        $rObj = new RuntimeObject($class, $program);
        $rObj->setInternalAttribute($value);
        return $rObj;
    }

    /**
     * Sets the internal attribute to the given value
     *
     * @param mixed $value Any possible value of type int|string|true|false|null|Block
     */
    public function setInternalAttribute(mixed $value): void
    {
        $this->internal = $value;
    }

    public function __toString(): string
    {
        $internalAttr = $this->internal ?? 'null';

        return "[Runtime Object]\n  Class type: {$this->class}\n  Primitive type: {$this->primitiveType}\n"
            . "Internal attribute: {$internalAttr}\n";
    }
}
