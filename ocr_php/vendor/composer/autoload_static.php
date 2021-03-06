<?php

// autoload_static.php @generated by Composer

namespace Composer\Autoload;

class ComposerStaticInitfe0b38e7a9d3a688ea926d32fe765f34
{
    public static $prefixLengthsPsr4 = array (
        'A' => 
        array (
            'Alchemy\\Zippy\\' => 14,
        ),
    );

    public static $prefixDirsPsr4 = array (
        'Alchemy\\Zippy\\' => 
        array (
            0 => __DIR__ . '/..' . '/alchemy/zippy/src',
        ),
    );

    public static $prefixesPsr0 = array (
        'S' => 
        array (
            'Symfony\\Component\\Process\\' => 
            array (
                0 => __DIR__ . '/..' . '/symfony/process',
            ),
            'Symfony\\Component\\Filesystem\\' => 
            array (
                0 => __DIR__ . '/..' . '/symfony/filesystem',
            ),
        ),
        'D' => 
        array (
            'Doctrine\\Common\\Collections\\' => 
            array (
                0 => __DIR__ . '/..' . '/doctrine/collections/lib',
            ),
        ),
    );

    public static $classMap = array (
        'TesseractOCR' => __DIR__ . '/..' . '/thiagoalessio/tesseract_ocr/src/TesseractOCR.php',
    );

    public static function getInitializer(ClassLoader $loader)
    {
        return \Closure::bind(function () use ($loader) {
            $loader->prefixLengthsPsr4 = ComposerStaticInitfe0b38e7a9d3a688ea926d32fe765f34::$prefixLengthsPsr4;
            $loader->prefixDirsPsr4 = ComposerStaticInitfe0b38e7a9d3a688ea926d32fe765f34::$prefixDirsPsr4;
            $loader->prefixesPsr0 = ComposerStaticInitfe0b38e7a9d3a688ea926d32fe765f34::$prefixesPsr0;
            $loader->classMap = ComposerStaticInitfe0b38e7a9d3a688ea926d32fe765f34::$classMap;

        }, null, ClassLoader::class);
    }
}
