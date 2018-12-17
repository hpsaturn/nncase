﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NnCase.Designer.Menus;
using NnCase.Designer.Modules.ModelDesigner.Commands;

namespace NnCase.Designer.Modules.ModelDesigner
{
    public static class MenuDefinitions
    {
        public static MenuItemDefinition OpenGraphMenuItem = new CommandMenuItemDefinition<OpenGraphCommandDefinition>(
            MainMenu.MenuDefinictions.FileNewOpenMenuGroup, 2);
    }
}