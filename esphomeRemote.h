#include "esphome.h"
#include "esphomeRemotePlayer.h"
#include "esphomeRemoteService.h"
#include "esphomeRemoteLight.h"
#include "esphomeRemoteSensor.h"
#include "MenuGlobals.h"

#ifndef ESPHOMEREMOTE
#define ESPHOMEREMOTE

bool menuDrawing = false;

class DisplayUpdateImpl: public DisplayUpdateInterface {
  public: virtual void updateDisplay(bool force) {
    if(menuDrawing) {
      return;
    }
    menuDrawing = true;
    if (force) {
      id(my_display).update();
      if (!id(backlight).state) {
        id(backlight).turn_on();
      }
      return;
    }

    if (id(backlight).state) {
      switch (activeMenuState) {
      case sourcesMenu:
        return;
      default:
        id(my_display).update();
        break;
      }
    }
  }
};

auto displayUpdate = DisplayUpdateImpl();
auto * sceneGroup = new SceneGroupComponent();
auto * sensorGroup = new SensorGroupComponent();
auto * speakerGroup = new SonosSpeakerGroupComponent(displayUpdate);
auto * lightGroup = new LightGroupComponent(displayUpdate);
MenuTitle activeMenuTitle = MenuTitle("", "", NoMenuTitleState);

std::string textWrap(std::string text, unsigned per_line) {
  unsigned line_begin = 0;
  while (line_begin < text.size()) {
    const unsigned ideal_end = line_begin + per_line;
    unsigned line_end = ideal_end <= text.size() ? ideal_end : text.size() - 1;

    if (line_end == text.size() - 1)
      ++line_end;
    else if (std::isspace(text[line_end])) {
      text[line_end] = '\n';
      ++line_end;
    } else {
      // backtrack
      unsigned end = line_end;
      while (end > line_begin && !std::isspace(text[end]))
        --end;

      if (end != line_begin) {
        line_end = end;
        text[line_end++] = '\n';
      } else
        text.insert(line_end++, 1, '\n');
    }

    line_begin = line_end;
  }

  return text;
}

MenuTitle menuTitleForType(MenuStates stringType) {
  switch (stringType) {
  case nowPlayingMenu:
    return MenuTitle("Now Playing", "", ArrowMenuTitleState);
  case sourcesMenu:
    return MenuTitle("Sources", "", ArrowMenuTitleState);
  case backlightMenu:
    return MenuTitle("Backlight", "", NoMenuTitleState);
  case sleepMenu:
    return MenuTitle("Sleep", "", NoMenuTitleState);
  case mediaPlayersMenu:
    return MenuTitle("Media Players", "", ArrowMenuTitleState);
  case lightsMenu:
    return MenuTitle("Lights", "", ArrowMenuTitleState);
  case scenesMenu:
    return MenuTitle("Scenes and Actions", "", ArrowMenuTitleState);
  case rootMenu:
    return MenuTitle("Home", "", NoMenuTitleState);
  case groupMenu:
    return MenuTitle("Speaker Group", "", ArrowMenuTitleState);
  case sensorsMenu:
    return MenuTitle("Sensors", "", ArrowMenuTitleState);
  }
  return MenuTitle("", "", NoMenuTitleState);
}

int drawPlayPauseIcon() {
  int yPos = id(header_height) / 2 - 6;
  int xPos = id(my_display).get_width() - batteryWidth - 16 - 4;
  switch(speakerGroup->activePlayer->menuTitlePlayerState()) {
    case PlayingMenuTitleState:
      id(my_display).image(xPos, yPos, & id(image_play));
      break;
    case PausedMenuTitleState:
      id(my_display).image(xPos, yPos, & id(image_pause));
      break;
    case StoppedMenuTitleState:
      id(my_display).image(xPos, yPos, & id(image_stop));
      break;
    default:
      break;
  }
  return xPos;
}

void drawCurrentMediaPlayer() {
  if (speakerGroup -> activePlayer -> entityId != "") {
    std::string entityId = speakerGroup -> activePlayer -> friendlyName;
    if (speakerGroup -> activePlayer -> playerType != TVRemotePlayerType) {
      SonosSpeakerComponent * activeSpeaker = static_cast < SonosSpeakerComponent * > (speakerGroup -> activePlayer);
      if (activeSpeaker != NULL) {
        if (activeSpeaker -> groupMembers.size() > 0) {
          entityId = entityId + " + " + to_string(activeSpeaker -> groupMembers.size());
        }
      }
    }
    id(my_display).printf(2, 0, & id(small_font), id(my_white), "%s", entityId.c_str());
  }
}

void drawBattery() {
  int batteryHeight = id(header_height) - 5;
  int yPos = 2;
  int xPos = 4;
  int capHeight = 6;
  int capWidth = 3;
  id(my_display).rectangle(id(my_display).get_width() - xPos - batteryWidth, yPos, batteryWidth, batteryHeight, id(my_gray_dark));
  id(my_display).rectangle(id(my_display).get_width() - xPos - 1, yPos + (batteryHeight / 2) - (capHeight / 2), capWidth, capHeight, id(my_gray_dark));
  if (id(TTGoBatteryPercent).state < 100) {
    id(my_display).filled_rectangle(id(my_display).get_width() - xPos - batteryWidth + 1, yPos + 1, (batteryWidth * id(TTGoBatteryPercent).state * 0.01) - 2, batteryHeight - 2, id(my_green));
  } else {
    id(my_display).filled_rectangle(id(my_display).get_width() - xPos - batteryWidth + 1, yPos + 1, batteryWidth - 2, batteryHeight - 2, id(my_yellow));
  }
}

void drawVolumeLevel(int oldXPos) {
  int xPos = oldXPos - 6;
  id(my_display).printf(xPos, 0, & id(small_font), id(my_white), TextAlign::TOP_RIGHT, "%.0f%%", speakerGroup -> getVolumeLevel());
}

int drawShuffle(int oldXPos) {
  if (speakerGroup -> activePlayer -> playerType == TVRemotePlayerType) {
    return oldXPos;
  }
  if (speakerGroup -> activePlayer -> playerState != StoppedRemoteState) {
    int xPos = oldXPos - 18;
    int yPos = id(header_height) / 2 - 6;
    if (speakerGroup -> mediaShuffling()) {
      id(my_display).image(xPos, yPos, & id(image_shuffle));
    } else {
      id(my_display).image(xPos, yPos, & id(image_shuffle_disabled));
    }
    return xPos;
  }
  return oldXPos;
}

void drawHeaderTitleWithString(std::string title) {
  id(my_display).printf(2, 0, & id(small_font), id(my_white), title.c_str());
}

void drawHeaderTitle() {
  switch (activeMenuState) {
  case rootMenu:
  case backlightMenu:
  case sleepMenu:
  case nowPlayingMenu:
    drawCurrentMediaPlayer();
    break;
  case sourcesMenu:
    drawHeaderTitleWithString("Sources");
    break;
  case groupMenu:
    drawHeaderTitleWithString("Group Speakers");
    break;
  case mediaPlayersMenu:
    drawHeaderTitleWithString("Media Players");
    break;
  case scenesMenu:
    drawHeaderTitleWithString("Scenes/Actions");
    break;
  case lightsMenu:
    drawHeaderTitleWithString("Lights");
    break;
  case sensorsMenu:
    drawHeaderTitleWithString("Sensors");
    break;
  }
}

void drawHeader() {
  id(my_display).rectangle(0, id(header_height), id(my_display).get_width(), 1, id(my_blue));
  drawBattery();
  drawHeaderTitle();
  drawVolumeLevel(drawShuffle(drawPlayPauseIcon()));
}

void drawTitle(int menuState, int i, std::string title, int yPos, bool buttonSpace) {
  int xPos = buttonSpace ? 20 : 4;
  if (menuState == i) {
    id(my_display).filled_rectangle(0, yPos, id(my_display).get_width(), id(medium_font_size) + id(margin_size), id(my_blue));
    id(my_display).printf(xPos, yPos, & id(medium_font), id(my_white), TextAlign::TOP_LEFT, "%s", title.c_str());
  } else {
    id(my_display).printf(xPos, yPos, & id(medium_font), id(my_white), TextAlign::TOP_LEFT, "%s", title.c_str());
  }
}

int maxItems() {
  int maxItems = ((id(my_display).get_height() - id(header_height)) / (id(medium_font_size) + id(margin_size))) - 1;
  return maxItems;
}

void drawScrollBar(int menuTitlesCount, int headerHeight) {
  int scrollBarWidth = 4;
  int scrollBarMargin = 2;
  if (menuTitlesCount > maxItems() + 1) {
    double screenHeight = id(my_display).get_height() - headerHeight;
    double height = maxItems() * (screenHeight / menuTitlesCount);
    double yPos = (((screenHeight - height) / (menuTitlesCount - 1)) * menuIndex) + 1 + headerHeight;
    id(my_display).filled_rectangle(id(my_display).get_width() - scrollBarWidth - scrollBarMargin, headerHeight, scrollBarWidth + scrollBarMargin, screenHeight, id(my_gray_dark_2));
    id(my_display).filled_rectangle(id(my_display).get_width() - scrollBarWidth, yPos, scrollBarWidth, height - 1, id(my_blue));
  }
}

void drawSwitch(bool switchState, int yPos) {
  id(my_display).circle(8, yPos + (id(medium_font_size) + id(margin_size)) / 2, 7, id(my_white));
  if (switchState) {
    id(my_display).filled_circle(8, yPos + (id(medium_font_size) + id(margin_size)) / 2, 5, id(my_white));
  }
}

void drawArrow(int yPos) {
  int xPos = id(my_display).get_width() - 8;
  id(my_display).line(xPos, yPos + 4, xPos + 3, yPos + (id(medium_font_size) + id(margin_size)) / 2, id(my_white));
  id(my_display).line(xPos, yPos + (id(medium_font_size) + id(margin_size)) - 4, xPos + 3, yPos + (id(medium_font_size) + id(margin_size)) / 2, id(my_white));
}

void scrollMenuPosition() {
  int menuState = menuIndex;

  if (menuState - maxItems() > scrollTop) {
    scrollTop = menuState - maxItems();
    // menu down
  } else if (menuState - scrollTop < 0) {
    scrollTop = menuState;
    // menu up
  }
}

void drawTitleImage(int characterCount, int yPos, MenuTitleState titleState, bool selected) {
  int imageHeight = 12;
  int adjustedYPos = yPos + (id(medium_font_size) / 4);
  int xPos = ((characterCount + 1) * (id(medium_font_size) * 0.6)) + 4;
  switch(titleState) {
    case PlayingMenuTitleState:
      if(selected) {
        id(my_display).image(xPos, adjustedYPos, &id(image_play_white));
      } else {
        id(my_display).image(xPos, adjustedYPos, &id(image_play));
      }
      break;
    case PausedMenuTitleState:
      if(selected) {
        id(my_display).image(xPos, adjustedYPos, &id(image_pause_white));
      } else {
        id(my_display).image(xPos, adjustedYPos, &id(image_pause));
      }
      break;
    case StoppedMenuTitleState:
      if(selected) {
        id(my_display).image(xPos, adjustedYPos, &id(image_stop_white));
      } else {
        id(my_display).image(xPos, adjustedYPos, &id(image_stop));
      }
      break;
    default:
      break;
  }
}

void drawGroupedBar(int yPos, bool extend) {
  int xPos = 8;
  int width = 8;
  int lineHeight = extend ? id(medium_font_size) + id(margin_size) : (id(medium_font_size) + id(margin_size)) / 2;
  id(my_display).line(xPos, yPos, xPos, yPos + lineHeight, id(my_white));
  id(my_display).line(xPos, yPos + (id(medium_font_size) + id(margin_size)) / 2, xPos + width, yPos + (id(medium_font_size) + id(margin_size)) / 2, id(my_white));
}

void drawMenu(std::vector <MenuTitle> menuTitles) {
  activeMenuTitleCount = menuTitles.size();
  if(menuTitles.size() == 0 ) {
    return;
  }
  scrollMenuPosition();
  int menuState = menuIndex;
  MenuTitle menuTitleCopy = menuTitles[menuIndex];
  activeMenuTitle = menuTitleCopy;
  for (int i = scrollTop; i < menuTitles.size(); i++) {
    if (i > scrollTop + maxItems()) {
      break;
    }
    int yPos = ((i - scrollTop) * (id(medium_font_size) + id(margin_size))) + id(header_height);
    drawTitle(menuState, i, menuTitles[i].friendlyName, yPos, menuTitles[i].indentLine());
    switch(menuTitles[i].titleState) {
      case NoMenuTitleState:
        break;
      case OffMenuTitleState:
      case OnMenuTitleState:
        drawSwitch(menuTitles[i].titleState == OnMenuTitleState, yPos);
        break;
      case ArrowMenuTitleState:
        if(menuState == i) {
          drawArrow(yPos);
        }
        break;
      case PlayingMenuTitleState:
      case PausedMenuTitleState:
      case StoppedMenuTitleState:
        drawTitleImage(menuTitles[i].friendlyName.length(), yPos, menuTitles[i].titleState, menuState == i);
        break;
      case GroupedMenuTitleState:
        bool extend = i < menuTitles.size() && menuTitles[i+1].titleState == GroupedMenuTitleState;
        drawGroupedBar(yPos, extend);
        break;
    }
  }
  drawScrollBar(menuTitles.size(), id(header_height));
}

std::vector <MenuTitle> menuTypesToTitles(std::vector <MenuStates> menu) {
  std::vector <MenuTitle> out;
  for (auto & menuItem: menu) {
    out.push_back(menuTitleForType(menuItem));
  }
  return out;
}

std::vector <MenuTitle> activeMenu() {
  int x = menuIndex;
  switch (activeMenuState) {
  case rootMenu:
    return menuTypesToTitles(rootMenuTitles());
  case sourcesMenu:
    return speakerGroup -> activePlayer -> sources;
  case mediaPlayersMenu:
    return speakerGroup -> mediaPlayersTitleString();
  case scenesMenu:
    return sceneGroup -> sceneTitleStrings();
  case sensorsMenu:
    return sensorGroup -> sensorTitles();
  default:
    ESP_LOGD("WARNING", "menu is bad  %d", x);
    std::vector <MenuTitle> out;
    return out;
  }
}

void topMenu() {
  menuIndex = 0;
  speakerGroup->newSpeakerGroupParent = NULL;
  optionMenu = noOptionMenu;
  activeMenuState = MenuStates::rootMenu;
}

void idleMenu() {
  menuIndex = 0;
  speakerGroup->newSpeakerGroupParent = NULL;
  optionMenu = noOptionMenu;
  activeMenuState = MenuStates::nowPlayingMenu;
}

void idleTick() {
  if (id(idle_time) == 5) {
    optionMenu = noOptionMenu;
    displayUpdate.updateDisplay(true);
  } else if (id(idle_time) == 20) {
    if (speakerGroup -> playerSearchFinished) {
      idleMenu();
      displayUpdate.updateDisplay(true);
    }
    ESP_LOGD("idle", "turning off display");
    id(backlight).turn_off();
    // menuDrawing = false;
  } else if (id(idle_time) > 3600) {
    ESP_LOGD("idle", "night night");
    id(tt_sleep).turn_on();
    return;
  }
  id(idle_time) ++;
}

void drawTVOptionMenu() {
  id(my_display).circle(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.45 + 24, 48, id(my_gray));
  id(my_display).printf(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.15 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, "Remote Menu");
  id(my_display).printf(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.75 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, "Pause");
  id(my_display).printf(id(my_display).get_width() * 0.2, (id(my_display).get_height() - 16) * 0.45 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, "Back");
  id(my_display).printf(id(my_display).get_width() * 0.8, (id(my_display).get_height() - 16) * 0.45 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, "Home");
  id(my_display).printf(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.45 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, "TV Power");
}

void drawSpeakerOptionMenu() {
  id(my_display).circle(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.45 + 24, 48, id(my_gray));
  id(my_display).printf(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.15 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, speakerGroup -> shuffleString().c_str());
  id(my_display).printf(id(my_display).get_width() * 0.5, (id(my_display).get_height() - 16) * 0.75 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, "Group");
  id(my_display).printf(id(my_display).get_width() * 0.8, (id(my_display).get_height() - 16) * 0.45 + 16, & id(small_font), id(my_white), TextAlign::TOP_CENTER, speakerGroup -> muteString().c_str());
}

void drawVolumeOptionMenu() {
  int margin = 18;
  int yPos = id(my_display).get_height() - 10;
  int barWidth = (id(my_display).get_width() - (margin * 2) - 2) * (speakerGroup -> getVolumeLevel() / 100);
  id(my_display).image(4, yPos - 1, & id(image_volume_low));
  id(my_display).image(id(my_display).get_width() - 14, yPos - 1, & id(image_volume_high));
  id(my_display).rectangle(margin, yPos, id(my_display).get_width() - margin * 2, 10, id(my_blue));
  id(my_display).filled_rectangle(margin + 2, yPos + 2, barWidth, 6, id(my_blue));
}

int drawTextWrapped(int xPos, int yPos, int fontSize, Font * font, Color color, std::string text, int characterLimit) {
  std::vector <std::string> output;
  std::string wrappedTitles = textWrap(text, characterLimit);
  tokenize(wrappedTitles, "\n", output);
  for (int i = 0; i < output.size(); i++) {
    id(my_display).printf(xPos, yPos + (i * id(medium_font_size)), font, color, TextAlign::TOP_CENTER, output[i].c_str());
  }
  return yPos + (output.size() * id(medium_font_size));
}

bool drawOptionMenuAndStop() {
  switch (optionMenu) {
  case tvOptionMenu:
    drawTVOptionMenu();
    return true;
  case speakerOptionMenu:
    drawSpeakerOptionMenu();
    return true;
  case volumeOptionMenu:
    // called later so it's over text
    return false;
  case noOptionMenu:
    return false;
  case playingNewSourceMenu:
    id(my_display).printf(id(my_display).get_width() / 2, 20 + id(margin_size), & id(medium_font), id(my_white), TextAlign::TOP_CENTER, "Playing...");
    drawTextWrapped(id(my_display).get_width() / 2, 40, 24, & id(large_font), id(my_white), playingNewSourceText.c_str(), 16);
    return true;
  }
  return true;
}

void drawNowPlaying() {
  if (drawOptionMenuAndStop()) {
    return;
  }
  int yPos = id(header_height) + 24;
  if(speakerGroup->activePlayer->playerState == PowerOffRemoteState) {
    id(my_display).printf(id(my_display).get_width() / 2, yPos, & id(large_font), id(my_white), TextAlign::TOP_CENTER, "Power Off");
    return;
  }
  id(my_display).printf(4, id(header_height), & id(medium_font), id(my_white), TextAlign::TOP_LEFT, "Now Playing,");
  if (speakerGroup -> activePlayer -> mediaArtist == "" && speakerGroup -> activePlayer -> mediaTitle == "") {
    id(my_display).printf(id(my_display).get_width() / 2, yPos, & id(large_font), id(my_white), TextAlign::TOP_CENTER, "Nothing!");
    return;
  }
  int artistTextHeight = yPos;
  if (speakerGroup -> activePlayer -> mediaArtist != "") {
    artistTextHeight = drawTextWrapped(id(my_display).get_width() / 2, yPos, 24, & id(large_font), id(my_white), speakerGroup -> activePlayer -> mediaArtist, 16);
  }
  if (speakerGroup -> activePlayer -> mediaTitle != "") {
    drawTextWrapped(id(my_display).get_width() / 2, artistTextHeight + id(margin_size), 15, & id(medium_font), id(my_white), speakerGroup -> activePlayer -> mediaTitle, 25);
  }
  if (optionMenu == volumeOptionMenu) {
    drawVolumeOptionMenu();
  }
}

void drawMenu() {
  if (speakerGroup -> playerSearchFinished == false) {
    id(my_display).printf(40, 40, & id(large_font), id(my_blue), TextAlign::TOP_LEFT, "beep boop");
    speakerGroup -> findActivePlayer();
    menuDrawing = false;
    return;
  }
  switch (activeMenuState) {
  case nowPlayingMenu:
    drawNowPlaying();
    break;
  case sourcesMenu:
    drawMenu(activeMenu());
    break;
  case lightsMenu:
    drawMenu(lightGroup -> lightTitleSwitches());
    break;
  case groupMenu:
    if (speakerGroup -> newSpeakerGroupParent != NULL) {
      drawMenu(speakerGroup -> groupTitleSwitches());
      break;
    }
    drawMenu(speakerGroup -> groupTitleString());
    break;
  default:
    drawMenu(activeMenu());
    break;
  }
  drawHeader();
  menuDrawing = false;
}

void selectMediaPlayers() {
  if(activeMenuTitle.entityId == speakerGroup->tv->entityId) {
    speakerGroup->activePlayer = speakerGroup->tv;
  } else {
    for (auto & speaker: speakerGroup->speakers) {
      if(speaker->entityId == activeMenuTitle.entityId) {
        speakerGroup->activePlayer = speaker;
        break;
      }
    }
  }
  topMenu();
}

bool selectRootMenu() {
  MenuStates currentMenu = rootMenuTitles()[menuIndex];
  switch (currentMenu) {
  case sourcesMenu:
    activeMenuState = sourcesMenu;
    break;
  case nowPlayingMenu:
    activeMenuState = nowPlayingMenu;
    break;
  case mediaPlayersMenu:
    activeMenuState = mediaPlayersMenu;
    break;
  case lightsMenu:
    activeMenuState = lightsMenu;
    break;
  case scenesMenu:
    activeMenuState = scenesMenu;
    break;
  case backlightMenu:
    topMenu();
    id(backlight).turn_off();
    return false;
  case sleepMenu:
    id(tt_sleep).turn_on();
    return false;
  case sensorsMenu:
    activeMenuState = sensorsMenu;
    break;
  case groupMenu:
  case rootMenu:
    ESP_LOGD("WARNING", "menu is bad  %d", menuIndex);
    return false;
  }
  menuIndex = 0;
  return true;
}

bool selectMenu() {
  int menuIndexForSource = menuIndex;
  switch (activeMenuState) {
  case rootMenu:
    return selectRootMenu();
  case nowPlayingMenu:
    activeMenuState = MenuStates::nowPlayingMenu;
    break;
  case sourcesMenu:
    idleMenu();
    speakerGroup -> activePlayer -> playSource(menuIndexForSource);
    optionMenu = playingNewSourceMenu;
    displayUpdate.updateDisplay(true);
    break;
  case groupMenu:
    speakerGroup -> selectGroup(&activeMenuTitle);
    break;
  case mediaPlayersMenu:
    selectMediaPlayers();
    break;
  case scenesMenu:
    if (sceneGroup -> selectScene(menuIndexForSource)) {
      topMenu();
    }
    break;
  case lightsMenu:
    if (lightGroup -> selectLight(menuIndexForSource)) {
      topMenu();
    }
    break;
  default:
    ESP_LOGD("WARNING", "menu state is bad but its an enum");
    return false;
  }
  return true;
}

bool buttonPressWakeUpDisplay() {
  if (id(idle_time) != -1) {
    id(idle_time) = 0;
  }
  if (!speakerGroup -> playerSearchFinished) {
    speakerGroup -> findActivePlayer();
  }
  if (!id(backlight).state) {
    id(backlight).turn_on();
    displayUpdate.updateDisplay(false);
    return true;
  }
  return false;
}

#endif