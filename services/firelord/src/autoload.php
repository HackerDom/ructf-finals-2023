<?php

spl_autoload_register(function () {
    $files = glob("./*/*.php");

    foreach ($files as $file)
    {
        include_once $file;
    }
});
