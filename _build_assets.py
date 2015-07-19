#!/usr/bin/python

import os
import sys
import time
import subprocess
import urllib

G_ROOT_DIR = "R:/SummerTraining";
G_BUILD_DIR = "../.Build/";
G_DEMOS_DIR = "../_Demos/";
G_ENGINE_DIR = "../_Engine/";
G_BINARIES_DIR = "../_Binaries/";

#shaderc_exe = os.path.join( G_ROOT_DIR, 'shaderc.exe' )
shaderc_exe = 'shaderc.exe'
input_folder = "R:/SummerTraining/_Demos/";
output_folder = "R:/SummerTraining/_Demos/runtime/shaders";


def quote( s ):
    return '"%s"' % s


def CompileShaderD3D11( folder, filename ):
        if filename.startswith('fs_'):
                shaderType = '"f"'
                shaderProfile = '"ps_4_0"'
        if filename.startswith('vs_'):
                shaderType = '"v"'
                shaderProfile = '"vs_4_0"'
        #print( 'Compiling ' + filename + ' in ' + folder )

        filebase, fileext = os.path.splitext( filename )

        input_path = os.path.join( folder, filename )
        input_path = os.path.normpath( input_path )

        output_filename = filebase + '.bin'

        output_path = os.path.join( output_folder, 'dx11', output_filename )
        output_path = os.path.normpath( output_path )

        #print( 'input_path = ' + input_path + ', output_path = ' + output_path )

        args = [ shaderc_exe,
                 '-f', quote(input_path),
                 '-o', quote(output_path), '--type', shaderType,
                 '-i', '"bgfx/src;bgfx/examples/common"',
                 '--platform', '"windows"',
                 '--profile', shaderProfile,
                 '-o3'
                 ]

        cmd = ''
        for arg in args:
                cmd += ' ' + arg
        
        print( cmd )

        os.chdir(G_ROOT_DIR);


        subprocess.call( args )
        #subprocess.call( shaderc_exe, '-f', input_path, '-o', output_path, '--type', shaderType, '-i', "bgfx/src;bgfx/examples/common", '--platform', "windows", '--profile', shaderProfile, '-o3' )

        #status = os.system( cmd )
        #print( "Status: ", status )

        #subprocess.call( cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
        #subprocess.call( cmd )

        #process = subprocess.Popen( cmd )
        #process.wait()

        #from subprocess import call
        #call( cmd )

        #time.sleep(3)

        #input("Press Enter to continue...")

        return


#os.system('"C:/Windows/System32/notepad.exe"')

for folder, subs, files in os.walk( "R:/SummerTraining/_Demos" ):
        for filename in files:
                if (not filename.startswith('varying.def')) and filename.endswith('sc'):
                        CompileShaderD3D11( folder, filename )
