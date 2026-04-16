--TEST--
Nested `when` rules (when-inside-then) actually execute
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// When `type == business`, require company. Additionally, if `region == EU`
// (nested when inside the business-branch), require a VAT number.
$v = new Validator([
    'type' => ['required', ['in', ['personal', 'business']]],
    'region' => ['required', 'string'],
    'company' => [
        ['when', ['type', '=', 'business'], [
            'required', 'string', ['min', 2],
        ]],
    ],
    'vat' => [
        ['when', ['type', '=', 'business'], [
            // Nested when — only require `vat` when region is EU.
            ['when', ['region', '=', 'EU'], [
                'required', 'string',
            ]],
        ]],
    ],
]);

// Business + EU + no vat → the nested when must fire and flag the missing vat.
$result = $v->validate([
    'type'    => 'business',
    'region'  => 'EU',
    'company' => 'Acme',
]);
var_dump($result->failed());
var_dump($result->hasError('vat'));

// Business + US + no vat → nested when's condition is false, should pass.
$result = $v->validate([
    'type'    => 'business',
    'region'  => 'US',
    'company' => 'Acme',
]);
var_dump($result->valid());

// Personal + EU + no vat → outer when's condition is false, should pass.
$result = $v->validate([
    'type'   => 'personal',
    'region' => 'EU',
]);
var_dump($result->valid());

// Business + EU + vat → should pass.
$result = $v->validate([
    'type'    => 'business',
    'region'  => 'EU',
    'company' => 'Acme',
    'vat'     => 'EU123456789',
]);
var_dump($result->valid());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
