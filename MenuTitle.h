
#ifndef MENUTITLE
#define MENUTITLE

enum MenuTitleState {
  NoMenuTitleState,
  OffMenuTitleState,
  OnMenuTitleState,
  ArrowMenuTitleState,
  PlayingMenuTitleState,
  PausedMenuTitleState,
  StoppedMenuTitleState,
  GroupedMenuTitleState
};

class MenuTitle {
  public:
    MenuTitle(std::string newFriendlyName, std::string newEntityId, MenuTitleState newTitleState) : friendlyName(newFriendlyName), entityId(newEntityId), titleState(newTitleState) { }
    std::string friendlyName;
    std::string entityId;
    MenuTitleState titleState;
    
    bool indentLine() {
      switch (titleState) {
        case OffMenuTitleState:
        case OnMenuTitleState:
        case GroupedMenuTitleState:
          return true;
        default:
          return false;
      }
      return false;
    }
};

#endif