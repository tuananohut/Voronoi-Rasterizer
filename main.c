#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <limits.h>

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
static int depth[HEIGHT][WIDTH]; 
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

void apply_next_seed(size_t seed_index)
{
  Point seed = seeds[seed_index];
  Color32 color = palette[seed_index%palette_count];
    for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
	{
	  int dx = x - seed.x;
	  int dy = y - seed.y;
	  int d = dx*dx + dy*dy;
	  if (d < depth[y][x])
	    {
	      depth[y][x] = d;
	      image[y][x] = color; 
	    }
	}
    }
}

void render_voronoi_interesting(void)
{
  for (int y = 0; y < HEIGHT; ++y)
    {
      for (int x = 0; x < WIDTH; ++x)
	{
	  depth[y][x] = INT_MAX; 
	}
    }

  for (size_t i = 0; i < SEEDS_COUNT; ++i)
    {
      apply_next_seed(i);
    }

  // render_seed_marker();
}

char *render_modes[] =
  {
    "euclidean",
    "manhattan",
    "interesting"
  };
#define render_mode_count (sizeof(render_modes)/sizeof(render_modes[0]))

char *save_modes[] =
  {
    "ppm",
    "mp4"
  };
#define save_mode_count (sizeof(save_modes)/sizeof(save_modes[0]))

int main(int argc, char *argv[]) 
{
  if (argc < 2)
    {
      printf("This is Voronoi Rasterizer. You have to write render and save mode to run it.\n");
      printf("Render modes: \n");
      for (size_t i = 0; i < render_mode_count; ++i)
	{
	  printf("%s\n", render_modes[i]);
	}
      printf("\nSave modes: \n");
      for (size_t i = 0; i < save_mode_count; ++i)
	{
	  printf("%s\n", save_modes[i]);
	}
      return -1; 
    }
  
  srand(time(0));
  fill_image(BACKGROUND_COLOR);
  generate_random_seeds();
  
  char *render_mode = argv[1];
  char *save_mode = argv[2];

  for (size_t i = 0; i < render_mode_count; ++i)
    {
      if (strcmp(render_mode, "euclidean") == 0)
	{
	  render_voronoi_euclidean();
	}
      else if (strcmp(render_mode, "manhattan") == 0)
	{
	  render_voronoi_manhattan();
	}
      else if (strcmp(render_mode, "interesting") == 0)
	{
	  render_voronoi_interesting();
	}
      else
	{
	  fprintf(stderr, "Unknown render mode: %s\n", render_mode);
	}
    }
  
  render_seed_marker();     

  for (size_t i = 0; i < save_mode_count; ++i)
    {
      if (strcmp(save_mode, "ppm") == 0)
	{
	  save_image_as_ppm(OUTPUT_FILE_PATH);
	}
      else if (strcmp(save_mode, "mp4") == 0)
	{
	  save_image_as_mp4(OUTPUT_FILE_PATH);
	  int ret = system("ffmpeg -framerate 30 -i output-%02d.ppm -c:v libx264 -pix_fmt yuv420p out.mp4");

	  if (ret != 0)
	    {
	      fprintf(stderr, "FFmpeg command failed!\n");
	    }
	}
      else
	{
	  fprintf(stderr, "Unknown save mode: %s\n", save_mode);
	}
    }

  return 0; 
}
