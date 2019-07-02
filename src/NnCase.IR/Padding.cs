﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace NnCase.IR
{
    [DebuggerDisplay("{{Before}, {After}}")]
    public struct Padding
    {
        public int Before { get; set; }

        public int After { get; set; }

        public int Sum => Before + After;
    }
}