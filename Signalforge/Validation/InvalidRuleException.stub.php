<?php
/**
 * Signalforge Validation Extension
 * InvalidRuleException.stub.php - IDE stub for the InvalidRuleException class
 *
 * @package Signalforge\Validation
 */

declare(strict_types=1);

namespace Signalforge\Validation;

/**
 * Thrown when a rule definition supplied to {@see Validator} is malformed.
 *
 * Triggered during rule parsing (in the constructor or {@see Validator::make()})
 * or when {@see Validator::validate()} is called on an improperly initialized
 * instance.
 */
class InvalidRuleException extends \Exception
{
}
