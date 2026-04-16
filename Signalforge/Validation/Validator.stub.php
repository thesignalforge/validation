<?php
/**
 * Signalforge Validation Extension
 * Validator.stub.php - IDE stub for the Validator class
 *
 * @package Signalforge\Validation
 */

declare(strict_types=1);

namespace Signalforge\Validation;

/**
 * Rule-based array validator implemented in C.
 *
 * Rules are described as a map from field path to either a pipe-delimited
 * string ("required|string|min:3") or a list of rule strings. Wildcards
 * (`*`) are supported in field paths to validate every element of a
 * collection.
 *
 * Supported rule families (see phpinfo() for the full list):
 *  - Presence: required, nullable, filled, present
 *  - Types: string, integer, numeric, boolean, array
 *  - String: min, max, between, regex, alpha, alpha_num, alpha_dash
 *  - Comparison: gt, gte, lt, lte, in, not_in, same, different, confirmed
 *  - Format: email, url, ip, uuid, json, date, date_format
 *  - Regional: oib, phone, iban, vat_eu
 *  - Conditional: when
 *
 * @example
 * $validator = new Validator([
 *     'email' => 'required|email',
 *     'password' => ['required', 'string', 'min:8'],
 *     'tags.*' => 'string|alpha_dash',
 * ]);
 *
 * $result = $validator->validate($_POST);
 * if ($result->failed()) {
 *     foreach ($result->errors() as $field => $messages) {
 *         // ...
 *     }
 * }
 */
final class Validator
{
    /**
     * Build a validator from a rule definition.
     *
     * The rules array is parsed once and cached on the instance, so calling
     * {@see validate()} multiple times is cheap.
     *
     * @param array<string, string|array<int, string>> $rules Field => rule(s)
     * @throws InvalidRuleException If a rule definition is malformed
     */
    public function __construct(array $rules) {}

    /**
     * Validate input data against the configured rules.
     *
     * Returns an immutable {@see ValidationResult} describing whether the
     * data passed, the per-field error messages, and the subset of fields
     * that successfully validated.
     *
     * @param array<string, mixed> $data Input data to validate
     * @return ValidationResult
     */
    public function validate(array $data): ValidationResult {}

    /**
     * Static factory mirroring the Laravel-style API.
     *
     * Builds a Validator from `$rules`. The `$data` argument is accepted
     * for API compatibility but is not stored — call {@see validate()}
     * yourself with the data when you are ready to run validation.
     *
     * @param array<string, mixed> $data Input data (not stored)
     * @param array<string, string|array<int, string>> $rules Field => rule(s)
     * @return Validator
     * @throws InvalidRuleException If a rule definition is malformed
     */
    public static function make(array $data, array $rules): Validator {}
}
