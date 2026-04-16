<?php
/**
 * Signalforge Validation Extension
 * ValidationResult.stub.php - IDE stub for the ValidationResult class
 *
 * @package Signalforge\Validation
 */

declare(strict_types=1);

namespace Signalforge\Validation;

/**
 * Immutable result returned from {@see Validator::validate()}.
 *
 * Holds the validity flag, per-field error messages, and the subset of
 * input fields that successfully validated.
 *
 * @example
 * $result = $validator->validate($input);
 *
 * if ($result->valid()) {
 *     $clean = $result->validated();
 * } else {
 *     foreach ($result->errors() as $field => $messages) {
 *         // render messages…
 *     }
 * }
 */
final class ValidationResult
{
    /**
     * Whether all rules passed.
     *
     * @return bool True if no errors were collected
     */
    public function valid(): bool {}

    /**
     * Whether any rule failed.
     *
     * @return bool True if at least one error was collected
     */
    public function failed(): bool {}

    /**
     * All collected error messages keyed by field name.
     *
     * @return array<string, array<int, string>> Field => list of messages
     */
    public function errors(): array {}

    /**
     * The subset of input fields that successfully validated.
     *
     * @return array<string, mixed>
     */
    public function validated(): array {}

    /**
     * Errors recorded for a single field.
     *
     * @param string $field Field name (or expanded wildcard path)
     * @return array<int, string> List of error messages, empty if none
     */
    public function errorsFor(string $field): array {}

    /**
     * Whether a specific field has at least one error.
     *
     * @param string $field Field name (or expanded wildcard path)
     * @return bool
     */
    public function hasError(string $field): bool {}
}
