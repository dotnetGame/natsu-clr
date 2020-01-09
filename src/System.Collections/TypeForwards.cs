using Natsu;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

[assembly: TypeForwardedTo(typeof(List<>))]
[assembly: TypeForwardedTo(typeof(Comparer<>))]
[assembly: TypeForwardedTo(typeof(EqualityComparer<>))]
[assembly: AssemblyEmbeddedCode("namespace System { namespace Collections { namespace Generic { template <class T0> using List_1_Enumerator = ::System_Private_CorLib::System::Collections::Generic::List_1_Enumerator<T0>; } } }")]