#include <pebble.h>

#define BUFFER_SIZE 12

static Window *window;

static TextLayer *line1;
static TextLayer *line2;
static TextLayer *line3;

static struct tm fake_time;

static char line1Str[2][BUFFER_SIZE];
static char line2Str[2][BUFFER_SIZE];
static char line3Str[2][BUFFER_SIZE];

static bool debug_flag = false;
static int splash_timer = 5;
static GBitmap *splash_image;
static BitmapLayer *splash_layer;
static GBitmap *blockp_image;
static BitmapLayer *blockp_layer;

static bool show_blockp = false;



static void click_config_provider(void *context);
static void configure_bold_layer(TextLayer *textlayer);
static void configure_light_layer(TextLayer *textlayer);
static void display_time(struct tm *t);
static void down_single_repeating_click_handler(ClickRecognizerRef recognizer, void *context);
static void handle_accel_tap(AccelAxisType axis, int32_t direction);
static void handle_deinit(void);
static void handle_init(void);
static void handle_second_tick(struct tm *tick, TimeUnits units_changed);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_release_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void set_bitmap_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint this_origin);
static void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length);
static void up_single_repeating_click_handler(ClickRecognizerRef recognizer, void *context);



static void click_config_provider(void *context)
{
   window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_single_repeating_click_handler);
   window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_single_repeating_click_handler);
   window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
   window_long_click_subscribe(BUTTON_ID_SELECT, 250, select_long_click_handler, select_long_release_handler);
}  // click_config_provider()


static void configure_bold_layer(TextLayer *textlayer)
{
   Layer *window_layer = window_get_root_layer(window);

   text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
   text_layer_set_text_color(textlayer, GColorWhite);
   text_layer_set_background_color(textlayer, GColorBlack);
   text_layer_set_text_alignment(textlayer, GTextAlignmentLeft);
   layer_add_child(window_layer, text_layer_get_layer(textlayer));

}  // configure_bold_layer()


static void configure_light_layer(TextLayer *textlayer)
{
   Layer *window_layer = window_get_root_layer(window);

   text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
   text_layer_set_text_color(textlayer, GColorWhite);
   text_layer_set_background_color(textlayer, GColorBlack);
   text_layer_set_text_alignment(textlayer, GTextAlignmentLeft);
   layer_add_child(window_layer, text_layer_get_layer(textlayer));
}  // configure_light_layer()


static void display_time(struct tm *t)
{
   time_to_3words(t->tm_hour, t->tm_min, line1Str[0], line2Str[0], line3Str[0], BUFFER_SIZE);

   if (strcmp(line1Str[0], line1Str[1]))
   {
      text_layer_set_text(line1, line1Str[0]);
      strcpy(line1Str[1], line1Str[0]);
   }
   if (strcmp(line2Str[0], line2Str[1]))
   {
      text_layer_set_text(line2, line2Str[0]);
      strcpy(line2Str[1], line2Str[0]);
   }
   if (strcmp(line3Str[0], line3Str[1]))
   {
      text_layer_set_text(line3, line3Str[0]);
      strcpy(line3Str[1], line3Str[0]);
   }
}  // display_time()


static void down_single_repeating_click_handler(ClickRecognizerRef recognizer, void *context)
{
   if (debug_flag == true)
   {
      fake_time.tm_min += 1;
      if (fake_time.tm_min >= 60)
      {
         fake_time.tm_min = 0;
         fake_time.tm_hour += 1;

         if (fake_time.tm_hour >= 24)
         {
            fake_time.tm_hour = 0;
         }
      }
      display_time(&fake_time);
   }
}  // down_single_repeating_click_handler()


static void handle_accel_tap(AccelAxisType axis, int32_t direction)
{
   if (debug_flag == 0)
   {
      show_blockp = true;
   }
}  // handle_accel_tap()


static void handle_deinit(void)
{
   tick_timer_service_unsubscribe();
   accel_tap_service_unsubscribe();

   text_layer_destroy(line1);
   text_layer_destroy(line2);
   text_layer_destroy(line3);

   bitmap_layer_destroy(blockp_layer);
   gbitmap_destroy(blockp_image);

   bitmap_layer_destroy(splash_layer);
   gbitmap_destroy(splash_image);

   window_destroy(window);
}  // handle_deinit()


static void handle_init(void)
{
   GRect dummy_frame = { {0, 0}, {0, 0} };

   time_t t = time(NULL);
   struct tm *time = localtime(&t);

   window = window_create();
   if (window == NULL)
   {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "...couldn't allocate window memory...");
      return;
   }

   window_set_fullscreen(window, true);
   window_stack_push(window, true /* Animated */);
   Layer *window_layer = window_get_root_layer(window);

   blockp_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLOCKP);
   blockp_layer = bitmap_layer_create(dummy_frame);
   bitmap_layer_set_bitmap(blockp_layer, blockp_image);

   window_set_click_config_provider(window, click_config_provider);

   strcpy(line1Str[0], "");
   strcpy(line1Str[1], "");
   strcpy(line2Str[0], "");
   strcpy(line2Str[1], "");
   strcpy(line3Str[0], "");
   strcpy(line3Str[1], "");

   // 1st line layers
   line1 = text_layer_create(GRect(0, 0, 144, 168));
   configure_bold_layer(line1);

   // 2nd layers
   line2 = text_layer_create(GRect(0, 55, 144, 50));
   configure_light_layer(line2);

   // 3rd layers
   line3 = text_layer_create(GRect(0, 110, 144, 50));
   configure_light_layer(line3);

   splash_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SPLASH);
   splash_layer = bitmap_layer_create(dummy_frame);
   bitmap_layer_set_bitmap(splash_layer, splash_image);
   layer_add_child(window_layer, bitmap_layer_get_layer(splash_layer));
 
   set_bitmap_image(&splash_image, splash_layer, RESOURCE_ID_IMAGE_SPLASH, GPoint (0, 0));

   // Configure time on init
   display_time(time);

   accel_tap_service_subscribe(&handle_accel_tap);
   tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}  // handle_init()


static void handle_second_tick(struct tm *tick, TimeUnits units_changed)
{
   if (splash_timer > 0)
   {
      splash_timer--;

      if (splash_timer != 0)
      {
         return;
      }
      else
      {
         if (show_blockp)
         {
            show_blockp = false;

            layer_remove_from_parent(bitmap_layer_get_layer(blockp_layer));
         }
         else
         {
            layer_remove_from_parent(bitmap_layer_get_layer(splash_layer));
         }

         display_time(tick);
      }
   }
   else
   {
      if (debug_flag == false)
      {
         if ((tick->tm_sec == 59) || (show_blockp == true))
         {
            show_blockp = true;

            splash_timer = 3;

            Layer *window_layer = window_get_root_layer(window);

            layer_add_child(window_layer, bitmap_layer_get_layer(blockp_layer));
 
            set_bitmap_image(&blockp_image, blockp_layer, RESOURCE_ID_IMAGE_BLOCKP, GPoint (0, 0));
         }
      }
   }
}  // handle_second_tick()


static void select_long_click_handler(ClickRecognizerRef recognizer, void *context)
{
   time_t t = time(NULL);
   struct tm *time = localtime(&t);

   if (splash_timer == 0)
   {
      if (debug_flag == false)
      {
         fake_time.tm_hour = time->tm_hour;
         fake_time.tm_min = time->tm_min;
      }

      debug_flag = !debug_flag;

      if (debug_flag == false)
      {
         fake_time.tm_hour = time->tm_hour;
         fake_time.tm_min = time->tm_min;
      }

      display_time(&fake_time);
   }
}  // select_long_clock_handler()


static void select_long_release_handler(ClickRecognizerRef recognizer, void *context)
{
}  // select_long_release_handler()


static void select_single_click_handler(ClickRecognizerRef recognizer, void *context)
{
   if ((debug_flag == 0) && (splash_timer == 0))
   {
      show_blockp = true;
   }
}  // select_single_click_handler()


static void set_bitmap_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint this_origin)
{
   gbitmap_destroy(*bmp_image);

   *bmp_image = gbitmap_create_with_resource(resource_id);
   GRect frame = (GRect)
   {
      .origin = this_origin,
      .size = (*bmp_image)->bounds.size
   };
   bitmap_layer_set_compositing_mode(bmp_layer, GCompOpAssign);
   layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
   bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
}  // set_bitmap_image()


static void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length)
{
   strcpy(line1, "");
   strcpy(line2, "");
   strcpy(line3, "");

   switch (hours % 12)
   {
      case 0:
         strcpy(line1, "twelve");
         break;

      case 1:
         strcpy(line1, "one");
         break;

      case 2:
         strcpy(line1, "two");
         break;

      case 3:
         strcpy(line1, "three");
         break;

      case 4:
         strcpy(line1, "four");
         break;

      case 5:
         strcpy(line1, "five");
         break;

      case 6:
         strcpy(line1, "six");
         break;

      case 7:
         strcpy(line1, "seven");
         break;

      case 8:
         strcpy(line1, "eight");
         break;

      case 9:
         strcpy(line1, "nine");
         break;

      case 10:
         strcpy(line1, "ten");
         break;

      case 11:
         strcpy(line1, "eleven");
         break;
   }

   // capitalize the first letter to indicate debug mode
   if (debug_flag)
   {
      line1[0] &= ~0x20;
   }

   if (minutes == 0)
   {
      strcpy(line2, "o'clock");
   }
   else // minutes 01-59
   {
      if (minutes < 10)
      {
         strcpy(line2, "oh");
         switch(minutes)
         {
            case 1:
               strcpy(line3, "one");
               break;

            case 2:
               strcpy(line3, "two");
               break;

            case 3:
               strcpy(line3, "three");
               break;

            case 4:
               strcpy(line3, "four");
               break;

            case 5:
               strcpy(line3, "five");
               break;

            case 6:
               strcpy(line3, "six");
               break;

            case 7:
               strcpy(line3, "seven");
               break;

            case 8:
               strcpy(line3, "eight");
               break;

            case 9:
               strcpy(line3, "nine");
               break;

         }
      }
      else // minutes 10-59
      {
         if (minutes < 13)
         {
            switch (minutes)
            {
               case 10:
                  strcpy(line2, "ten");
                  break;

               case 11:
                  strcpy(line2, "eleven");
                  break;

               case 12:
                  strcpy(line2, "twelve");
                  break;
            }
         }
         else // minutes 13-59
         {
            if (minutes < 20)
            {
               switch (minutes)
               {
                  case 13:
                     strcpy(line2, "thir");
                     break;

                  case 14:
                     strcpy(line2, "four");
                     break;

                  case 15:
                     strcpy(line2, "fif");
                     break;

                  case 16:
                     strcpy(line2, "six");
                     break;

                  case 17:
                     strcpy(line2, "seven");
                     break;

                  case 18:
                     strcpy(line2, "eight");
                     break;

                  case 19:
                     strcpy(line2, "nine");
                     break;
               }
               strcpy(line3, "teen");
            }
            else // minutes 20-59
            {
               switch (minutes / 10)
               {
                  case 2:
                     strcpy(line2, "twenty");
                     break;

                  case 3:
                     strcpy(line2, "thirty");
                     break;

                  case 4:
                     strcpy(line2, "forty");
                     break;

                  case 5:
                     strcpy(line2, "fifty");
                     break;
               }

               switch (minutes % 10)
               {
                  case 1:
                     strcpy(line3, "one");
                     break;

                  case 2:
                     strcpy(line3, "two");
                     break;

                  case 3:
                     strcpy(line3, "three");
                     break;

                  case 4:
                     strcpy(line3, "four");
                     break;

                  case 5:
                     strcpy(line3, "five");
                     break;

                  case 6:
                     strcpy(line3, "six");
                     break;

                  case 7:
                     strcpy(line3, "seven");
                     break;

                  case 8:
                     strcpy(line3, "eight");
                     break;

                  case 9:
                     strcpy(line3, "nine");
                     break;
               }
            }
         }
      }
   }
}  // time_to_3words()


static void up_single_repeating_click_handler(ClickRecognizerRef recognizer, void *context)
{
   if (debug_flag)
   {
      fake_time.tm_min -= 1;
      if (fake_time.tm_min < 0)
      {
         fake_time.tm_min = 59;
         fake_time.tm_hour -= 1;

         if (fake_time.tm_hour < 0)
         {
            fake_time.tm_hour = 23;
         }
      }
      display_time(&fake_time);
   }
}  // up_single_repeating_click_handler()


int main(void)
{
   handle_init();
   app_event_loop();
   handle_deinit();
}







