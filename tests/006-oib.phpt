--TEST--
OIB (Croatian Personal ID) validation
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

$v = new Validator(['oib' => ['required', 'oib']]);

// Valid OIBs
var_dump($v->validate(['oib' => '12345678903'])->valid());  // Valid checksum
var_dump($v->validate(['oib' => '00000000001'])->valid());  // Edge case

// Invalid OIBs
var_dump($v->validate(['oib' => '12345678900'])->failed());  // Wrong checksum
var_dump($v->validate(['oib' => '1234567890'])->failed());   // Too short
var_dump($v->validate(['oib' => '123456789012'])->failed()); // Too long
var_dump($v->validate(['oib' => '1234567890a'])->failed());  // Contains letter

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
