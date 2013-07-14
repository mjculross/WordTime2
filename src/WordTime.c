#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define BUFFER_SIZE 12

#define MY_UUID { 0x96, 0x18, 0x7e, 0xd3, 0x39, 0x79, 0x49, 0xe5, 0xba, 0x60, 0xc0, 0x13, 0x7d, 0xcd, 0x16, 0x6b }
PBL_APP_INFO(MY_UUID,
   "WordTIme", "KD5RXT",
   1, 0,
   RESOURCE_ID_IMAGE_MENU_ICON,
   APP_INFO_STANDARD_APP
);

Window window;

TextLayer line1;
TextLayer line2;
TextLayer line3;

PblTm fake_time;

static char line1Str[2][BUFFER_SIZE];
static char line2Str[2][BUFFER_SIZE];
static char line3Str[2][BUFFER_SIZE];

bool debug_flag = false;
int splash_timer = 5;
BmpContainer splash_image;



void click_config_provider(ClickConfig **config, Window *window);
void configure_bold_layer(TextLayer *textlayer);
void configure_light_layer(TextLayer *textlayer);
void display_time(PblTm *t);
void down_single_click_handler(ClickRecognizerRef recognizer, Window *window);
void handle_deinit(AppContextRef ctx);
void handle_init(AppContextRef ctx);
void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t);
void select_long_click_handler(ClickRecognizerRef recognizer, Window *window);
void select_long_release_handler(ClickRecognizerRef recognizer, Window *window);
void select_single_click_handler(ClickRecognizerRef recognizer, Window *window);
void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length);
void up_single_click_handler(ClickRecognizerRef recognizer, Window *window);



void click_config_provider(ClickConfig **config, Window *window)
{
   (void)window;

   config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
   config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

   config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
   config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;

   config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_single_click_handler;
   config[BUTTON_ID_SELECT]->click.repeat_interval_ms = 100;

   config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;
   config[BUTTON_ID_SELECT]->long_click.release_handler = (ClickHandler) select_long_release_handler;
} // click_config_provider()


void configure_bold_layer(TextLayer *textlayer)
{
   text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_GOTHAM_42_BOLD));
   text_layer_set_text_color(textlayer, GColorWhite);
   text_layer_set_background_color(textlayer, GColorClear);
   text_layer_set_text_alignment(textlayer, GTextAlignmentLeft);
} // configure_bold_layer()


void configure_light_layer(TextLayer *textlayer)
{
   text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_GOTHAM_42_LIGHT));
   text_layer_set_text_color(textlayer, GColorWhite);
   text_layer_set_background_color(textlayer, GColorClear);
   text_layer_set_text_alignment(textlayer, GTextAlignmentLeft);
} // configure_light_layer()


void display_time(PblTm *t)
{
   time_to_3words(t->tm_hour, t->tm_min, line1Str[0], line2Str[0], line3Str[0], BUFFER_SIZE);

   if (strcmp(line1Str[0], line1Str[1]))
   {
      text_layer_set_text(&line1, line1Str[0]);
      strcpy(line1Str[1], line1Str[0]);
   }
   if (strcmp(line2Str[0], line2Str[1]))
   {
      text_layer_set_text(&line2, line2Str[0]);
      strcpy(line2Str[1], line2Str[0]);
   }
   if (strcmp(line3Str[0], line3Str[1]))
   {
      text_layer_set_text(&line3, line3Str[0]);
      strcpy(line3Str[1], line3Str[0]);
   }
} // display_time()


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   (void)recognizer;
   (void)window;

   if (debug_flag)
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
} // down_single_click_handler()


void handle_deinit(AppContextRef ctx)
{
   (void)ctx;
}  // handle_deinit()


void handle_init(AppContextRef ctx)
{
   (void)ctx;

   window_init(&window, "TextWatch");
   window_stack_push(&window, true);
   window_set_fullscreen(&window, true);
   window_set_background_color(&window, GColorBlack);

   strcpy(line1Str[0], "");
   strcpy(line1Str[1], "");
   strcpy(line2Str[0], "");
   strcpy(line2Str[1], "");
   strcpy(line3Str[0], "");
   strcpy(line3Str[1], "");

   splash_timer = 5;

   // Init resources
   resource_init_current_app(&APP_RESOURCES);

   // 1st line layers
   text_layer_init(&line1, GRect(0, 18, 144, 50));
   configure_bold_layer(&line1);

   // 2nd layers
   text_layer_init(&line2, GRect(0, 55, 144, 50));
   configure_light_layer(&line2);

   // 3rd layers
   text_layer_init(&line3, GRect(0, 92, 144, 50));
   configure_light_layer(&line3);

   // Configure time on init
   get_time(&fake_time);
   display_time(&fake_time);

   // Load layers
   layer_add_child(&window.layer, &line1.layer);
   layer_add_child(&window.layer, &line2.layer);
   layer_add_child(&window.layer, &line3.layer);

   // Button functionality
   window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);

   bmp_init_container(RESOURCE_ID_IMAGE_SPLASH, &splash_image);
   layer_add_child(&window.layer, &splash_image.layer.layer);
} // handle_init()


void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t)
{
   (void)ctx;

   if (splash_timer > 0)
   {
      if (splash_timer == 1)
      {
         layer_remove_from_parent(&splash_image.layer.layer);
         bmp_deinit_container(&splash_image);
      }

      splash_timer--;

      if (splash_timer != 0)
      {
         return;
      }
      else
      {
         if (t->tick_time->tm_sec == 0)
         {
            if (debug_flag == false)
            {
               display_time(t->tick_time);
            }
            else
            {
               display_time(&fake_time);
            }
         }
      }
   }
} // handle_second_tick()


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   (void)recognizer;
   (void)window;

   debug_flag = !debug_flag;

   if (debug_flag == false)
   {
      get_time(&fake_time);
   }
   display_time(&fake_time);
} // select_long_clock_handler()


void select_long_release_handler(ClickRecognizerRef recognizer, Window *window)
{
   (void)recognizer;
   (void)window;
} // select_long_release_handler()


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   (void)recognizer;
   (void)window;
} // select_single_click_handler()


void time_to_3words(int hours, int minutes, char *line1, char *line2, char *line3, size_t length)
{
   strcpy(line1, "");
   strcpy(line2, "");
   strcpy(line3, "");

   switch (hours % 12)
   {
      case 0:
         if (debug_flag)
         {
            strcpy(line1, "Twelve");
         }
         else
         {
            strcpy(line1, "twelve");
         }
         break;

      case 1:
         if (debug_flag)
         {
            strcpy(line1, "One");
         }
         else
         {
            strcpy(line1, "one");
         }
         break;

      case 2:
         if (debug_flag)
         {
            strcpy(line1, "Two");
         }
         else
         {
            strcpy(line1, "two");
         }
         break;

      case 3:
         if (debug_flag)
         {
            strcpy(line1, "Three");
         }
         else
         {
            strcpy(line1, "three");
         }
         break;

      case 4:
         if (debug_flag)
         {
            strcpy(line1, "Four");
         }
         else
         {
            strcpy(line1, "four");
         }
         break;

      case 5:
         if (debug_flag)
         {
            strcpy(line1, "Five");
         }
         else
         {
            strcpy(line1, "five");
         }
         break;

      case 6:
         if (debug_flag)
         {
            strcpy(line1, "Six");
         }
         else
         {
            strcpy(line1, "six");
         }
         break;

      case 7:
         if (debug_flag)
         {
            strcpy(line1, "Seven");
         }
         else
         {
            strcpy(line1, "seven");
         }
         break;

      case 8:
         if (debug_flag)
         {
            strcpy(line1, "Eight");
         }
         else
         {
            strcpy(line1, "eight");
         }
         break;

      case 9:
         if (debug_flag)
         {
            strcpy(line1, "Nine");
         }
         else
         {
            strcpy(line1, "nine");
         }
         break;

      case 10:
         if (debug_flag)
         {
            strcpy(line1, "Ten");
         }
         else
         {
            strcpy(line1, "ten");
         }
         break;

      case 11:
         if (debug_flag)
         {
            strcpy(line1, "Eleven");
         }
         else
         {
            strcpy(line1, "eleven");
         }
         break;
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
} // time_to_3words()


void up_single_click_handler(ClickRecognizerRef recognizer, Window *window)
{
   (void)recognizer;
   (void)window;

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
} // up_single_click_handler()


void pbl_main(void *params)
{
   PebbleAppHandlers handlers =
   {
      .init_handler = &handle_init,
      .tick_info =
      {
         .tick_handler = &handle_second_tick,
         .tick_units = SECOND_UNIT
      }
   };
   app_event_loop(params, &handlers);
}

