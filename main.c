#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define WIDTH 1920
#define HEIGHT 1080
#define SEEDS_COUNT 9

#define OUTPUT_FILE_PATH "img.ppm"

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_MAGENTA 0xFFFF00FF
#define COLOR_RED 0xFF0000FF 
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFFFF0000

#define CYBERPUNK_WILD_STRAWBERRY 0xFF943DFF
#define CYBERPUNK_MEDIUM_RED_VIOLET 0xFF7E30B5
#define CYBERPUNK_DAISY_BUSH 0xFF982A6A
#define CYBERPUNK_METEORITE 0xFF6D1C3F
#define CYBERPUNK_VIOLET 0xFF4B0B21
#define CYBERPUNK_SAFFRON_MANGO 0xFF4EC5F9
#define CYBERPUNK_PERSIAN_GREEN 0xFFA1B300
#define CYBERPUNK_TEAL 0xFF807F00
#define CYBERPUBK_ORIENT 0xFF805B00

#define BACKGROUND_COLOR 0xFF181818

#define SEED_MARKER_RADIUS 5
#define SEED_MARKER_COLOR COLOR_BLACK

typedef uint32_t Color32;

typedef struct {
  int x, y; 
} Point; 

typedef struct {
  uint16_t x;
  uint16_t y; 
} Point32;

static Color32 image[HEIGHT][WIDTH];
static Point seeds[SEEDS_COUNT];
static Color32 palette[] = {
  CYBERPUNK_WILD_STRAWBERRY,
  CYBERPUNK_MEDIUM_RED_VIOLET,
  CYBERPUNK_DAISY_BUSH,
  CYBERPUNK_METEORITE,
  CYBERPUNK_VIOLET,
  CYBERPUNK_SAFFRON_MANGO,
  CYBERPUNK_PERSIAN_GREEN,
  CYBERPUNK_TEAL,
  CYBERPUBK_ORIENT,
};
#define palette_count (sizeof(palette)/sizeof(palette[0]))

void fill_image(Color32 color)
{
  for (size_t y = 0; y < HEIGHT; ++y)
    {
      for (size_t x = 0; x < WIDTH; ++x)
	{
	  image[y][x] = color; 
	}
    }
}

int sqr_dist(int x1, int y1, int x2, int y2)
{
  int dx = x1 - x2;
  int dy = y1 - y2;
  return dx*dx + dy*dy; 
}

int manhattan_dist(int x1, int y1, int x2, int y2)
{
  int dx = x1 - x2;
  int dy = y1 - y2;

  if (dx < 0)
    {
      dx *= -1;
    }

  if (dy < 0)
    {
      dy *= -1; 
    }
  
  return dx + dy; 
}

void fill_circle(int cx, int cy, int radius, uint32_t color)
{
  int x0 = cx - radius;
  int y0 = cy - radius;
  int x1 = cx + radius;
  int y1 = cy + radius;
  for (int x = x0; x <= x1; ++x)
    {
      if (0 <= x && x < WIDTH)
	{
	  for (int y = y0; y <= y1; ++y)
	    {
	      if (0 <= y && y < HEIGHT)
		{
		  if ((sqr_dist(cx, cy, x, y)) <= radius*radius)
		    {
		      image[y][x] = color; 
		    }
		}
	    }
	}
    }  
}

void save_image_as_ppm(const char *file_path)
{
  FILE *f = fopen(file_path, "wb");
  if (f == NULL)
    {
      fprintf(stderr, "ERROR: could write int file %s: %s\n", file_path, strerror(errno));
      exit(1);
    }
  
  fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT); 
  for (size_t y = 0; y < HEIGHT; ++y)
    {
      for (size_t x = 0; x < WIDTH; ++x)
	{
	  // 0xAABBGGRR
	  uint32_t pixel = image[y][x];
	  uint8_t bytes[3] =
	    {
	      (pixel&0x0000FF)>>8*0,
	      (pixel&0x00FF00)>>8*1,
	      (pixel&0xFF0000)>>8*2,
	    };

	  fwrite(bytes, sizeof(bytes), 1, f);
	  assert(!ferror(f));
	}
    }
  
  int ret = fclose(f);
  assert(ret == 0);
}

void save_image_as_mp4(const char *file_path)
{
  char buf[255];
  for (int i = 0; i < 60*10; ++i)
    {
      snprintf(buf, sizeof(buf), "output-%02d.ppm", i);
      file_path = buf;

      FILE *f = fopen(file_path, "wb");
      if (f == NULL)
	{
	  fprintf(stderr, "ERROR: could write int file %s: %s\n", file_path, strerror(errno));
	  exit(1);
	}
  
      fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT); 
      for (size_t y = 0; y < HEIGHT; ++y)
	{
	  for (size_t x = 0; x < WIDTH; ++x)
	    {
	      // 0xAABBGGRR
	      int yy = (y + i) % HEIGHT;
	      int xx = (x + i) % WIDTH;
	      uint32_t pixel = image[yy][xx];
	      uint8_t bytes[3] =
		{
		  (pixel&0x0000FF)>>8*0,
		  (pixel&0x00FF00)>>8*1,
		  (pixel&0xFF0000)>>8*2,
		};

	      fwrite(bytes, sizeof(bytes), 1, f);
	      assert(!ferror(f));
	    }
	}
  
      int ret = fclose(f);
      assert(ret == 0);      
    }
}

void generate_random_seeds(void)
{
  for (size_t i = 0; i < SEEDS_COUNT; ++i)
    {
      seeds[i].x = rand()%WIDTH;
      seeds[i].y = rand()%HEIGHT;
    }
}

void render_seed_marker(void)
{
  for (size_t i = 0; i < SEEDS_COUNT; ++i)
    {
      fill_circle(seeds[i].x, seeds[i].y, SEED_MARKER_RADIUS, SEED_MARKER_COLOR);
    }
  save_image_as_ppm(OUTPUT_FILE_PATH); 
}

void render_voronoi_euclidean(void)
{
  for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
	{
	  int j = 0; 
	  for (size_t i = 1; i < SEEDS_COUNT; ++i)
	    {
	      if (sqr_dist(seeds[i].x, seeds[i].y, x, y) < sqr_dist(seeds[j].x, seeds[j].y, x, y))
		{
		  j = i; 
		}
	    }
	  image[y][x] = palette[j%palette_count];
	}
    }
}

void render_voronoi_manhattan(void)
{
  for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
	{
	  int j = 0; 
	  for (size_t i = 1; i < SEEDS_COUNT; ++i)
	    {
	      if (manhattan_dist(seeds[i].x, seeds[i].y, x, y) < manhattan_dist(seeds[j].x, seeds[j].y, x, y))
		{
		  j = i; 
		}
	    }
	  image[y][x] = palette[j%palette_count];
	}
    }
}


Color32 point_to_color(Point p)
{
  assert(p.x >= 0);
  assert(p.y >= 0);
  assert(p.x < UINT16_MAX);
  assert(p.y < UINT16_MAX);
  uint16_t x = p.x; 
  uint16_t y = p.y; 
  return (y<<16) | x;
}

Point color_to_point(Color32 c)
{
  return (Point) {
    .x = (c&0x0000FFFF)>>0,
    .y = (c&0xFFFF0000)>>16
  };
  
}

void render_point_gradient(void)
{
  for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
	{
	  Point p = {x, y};
	  image[y][x] = point_to_color(p);
	}
    }
}

void apply_next_seed_color(Color32 next_seed_color)
{
  Point next_seed = color_to_point(next_seed_color); 
  for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
	{
	  Point curr_seed = color_to_point(image[y][x]);
	  if(sqr_dist(next_seed.x, next_seed.y, x, y) < sqr_dist(curr_seed.x, curr_seed.y, x, y))
	    {
	      image[y][x] = next_seed_color;  
	    }
	}
    }
}

void render_voronoi_interesting(void)
{
  fill_image(point_to_color(seeds[0]));
  for (size_t i = 1; i < SEEDS_COUNT; ++i)
    {
      apply_next_seed_color(point_to_color(seeds[i]));
    }
  render_seed_marker();
}

int main(void)
{
  // 0xAABBGGRR
  // RR GG BB AA
  srand(time(0));
  fill_image(BACKGROUND_COLOR);
  generate_random_seeds();
  render_voronoi_interesting(); 
  // render_seed_marker();
  // render_point_gradient(); 
  save_image_as_ppm(OUTPUT_FILE_PATH);
  // save_image_as_mp4(OUTPUT_FILE_PATH);
  return 0; 
}
