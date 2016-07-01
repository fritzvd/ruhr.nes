var gulp = require('gulp')
var child_process = require('child_process')
var nestmx = require('nes-tmx')
var slides = require('./slides.json')

var paths = {
  graphics: 'gfx/tiles.png',
  tilemap: 'gfx/*.tmx',
  slides: 'slides.json',
  src: 'src/main.c',
}

gulp.task('spriteBuild', createTiles)
gulp.task('nameTables', nameTables)
gulp.task('make', make)

gulp.task('dev', function() {
  gulp.watch(paths.graphics, ['spriteBuild']);
  gulp.watch(paths.tilemap, ['nameTables']);
  gulp.watch(paths.slides, ['nameTables', 'make']);
  gulp.watch(paths.src, ['make']);
})

function createTiles () {
  console.log('Making a tileset')
  var spritetiles = child_process.spawn(
    './tools/pilbmp2nes.py', ['-i', 'gfx/tiles.png', '-o', 'src/tiles.chr'])
    spritetiles.stdout.on('data', function (d) {
      console.log('' + d)
    })
    spritetiles.stderr.on('data', function (d) {
      console.error('' + d)
    })
  return spritetiles
}

function nameTables () {
  nestmx('gfx/title.tmx', 'src/title.h')
  slides.forEach(function (text, i){
    nestmx.slidemaker('gfx/slide.tmx', 'src/slide' + i + '.h', text)
  })
  // nestmx('gfx/slide.tmx', 'src/slide.h')
}

function make () {
  var make = child_process.spawn('make', {cwd: 'src'})

  make.stdout.on('data', function (d) {
    console.log('' + d)
  })
  make.stderr.on('data', function (d) {
    console.error('' + d)
  })
  return make;
}

gulp.task('build', ['nameTables', 'spriteBuild', 'make'])
gulp.task('default', ['build'])
