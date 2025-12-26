--TEST--
Conditional validation rules
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// Simple conditional - when type is 'business', company is required
$v = new Validator([
    'type' => ['required', ['in', ['personal', 'business']]],
    'company' => [
        ['when', ['type', '=', 'business'], [
            'required', 'string', ['min', 2]
        ]]
    ],
]);

// Business type without company - should fail
$result = $v->validate(['type' => 'business']);
var_dump($result->failed());
var_dump($result->hasError('company'));

// Business type with company - should pass
$result = $v->validate(['type' => 'business', 'company' => 'Acme Inc']);
var_dump($result->valid());

// Personal type without company - should pass
$result = $v->validate(['type' => 'personal']);
var_dump($result->valid());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
OK
