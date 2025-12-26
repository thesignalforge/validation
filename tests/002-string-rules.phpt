--TEST--
String validation rules
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// Min/Max
$v = new Validator(['name' => ['string', ['min', 3], ['max', 10]]]);
var_dump($v->validate(['name' => 'Jo'])->failed());
var_dump($v->validate(['name' => 'John'])->valid());
var_dump($v->validate(['name' => 'JohnJohnJohn'])->failed());

// Regex
$v = new Validator(['code' => ['string', ['regex', '/^[A-Z]{3}$/']]]);
var_dump($v->validate(['code' => 'ABC'])->valid());
var_dump($v->validate(['code' => 'abc'])->failed());
var_dump($v->validate(['code' => 'ABCD'])->failed());

// Alpha
$v = new Validator(['name' => ['string', 'alpha']]);
var_dump($v->validate(['name' => 'John'])->valid());
var_dump($v->validate(['name' => 'John123'])->failed());

// Alpha-num
$v = new Validator(['code' => ['string', 'alpha_num']]);
var_dump($v->validate(['code' => 'ABC123'])->valid());
var_dump($v->validate(['code' => 'ABC-123'])->failed());

// Starts with
$v = new Validator(['url' => ['string', ['starts_with', 'https://']]]);
var_dump($v->validate(['url' => 'https://example.com'])->valid());
var_dump($v->validate(['url' => 'http://example.com'])->failed());

// Ends with
$v = new Validator(['file' => ['string', ['ends_with', '.pdf']]]);
var_dump($v->validate(['file' => 'document.pdf'])->valid());
var_dump($v->validate(['file' => 'document.txt'])->failed());

// Contains
$v = new Validator(['desc' => ['string', ['contains', 'important']]]);
var_dump($v->validate(['desc' => 'This is important text'])->valid());
var_dump($v->validate(['desc' => 'This is other text'])->failed());

echo "OK\n";
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
OK
