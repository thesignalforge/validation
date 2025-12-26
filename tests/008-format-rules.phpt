--TEST--
Format validation rules
--SKIPIF--
<?php if (!extension_loaded('signalforge_validation')) die('skip'); ?>
--FILE--
<?php
use Signalforge\Validation\Validator;

// Email
$v = new Validator(['email' => ['email']]);
var_dump($v->validate(['email' => 'test@example.com'])->valid());
var_dump($v->validate(['email' => 'invalid'])->failed());
var_dump($v->validate(['email' => 'test@'])->failed());

// URL
$v = new Validator(['url' => ['url']]);
var_dump($v->validate(['url' => 'https://example.com'])->valid());
var_dump($v->validate(['url' => 'http://example.com'])->valid());
var_dump($v->validate(['url' => 'ftp://example.com'])->failed());

// IP
$v = new Validator(['ip' => ['ip']]);
var_dump($v->validate(['ip' => '192.168.1.1'])->valid());
var_dump($v->validate(['ip' => '::1'])->valid());
var_dump($v->validate(['ip' => 'invalid'])->failed());

// UUID
$v = new Validator(['uuid' => ['uuid']]);
var_dump($v->validate(['uuid' => '550e8400-e29b-41d4-a716-446655440000'])->valid());
var_dump($v->validate(['uuid' => 'invalid-uuid'])->failed());

// JSON
$v = new Validator(['data' => ['json']]);
var_dump($v->validate(['data' => '{"key": "value"}'])->valid());
var_dump($v->validate(['data' => '[1, 2, 3]'])->valid());
var_dump($v->validate(['data' => 'invalid json'])->failed());

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
OK
