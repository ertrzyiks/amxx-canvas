module.exports = function(grunt) {

    grunt.initConfig({
        pkg: grunt.file.readJSON('package.json'),
        amxmodx: {
            dist: {
                options:{
                    includePath: 'addons/amxmodx/scripting/include'
                },
                nonull: true,
                src: ['addons/amxmodx/scripting/*.sma'],
                dest: 'addons/amxmodx/plugins/'
            }
        }
    });

    grunt.loadNpmTasks('grunt-amxmodx');

    grunt.registerTask('default', ['test']);
    grunt.registerTask('test', ['amxmodx']);
};
