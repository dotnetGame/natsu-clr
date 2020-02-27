using Natsu;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

[assembly: TypeForwardedTo(typeof(Dictionary<,>))]
[assembly: TypeForwardedTo(typeof(List<>))]
[assembly: TypeForwardedTo(typeof(Comparer<>))]
[assembly: TypeForwardedTo(typeof(EqualityComparer<>))]
[assembly: AssemblyEmbeddedCode("namespace System { namespace Collections { namespace Generic { template <class T0> using List_1_Enumerator = ::System_Private_CoreLib::System::Collections::Generic::List_1_Enumerator<T0>; } } }")]
[assembly: AssemblyEmbeddedCode("namespace System { namespace Collections { namespace Generic { template <class T0, class T1> using Dictionary_2_Enumerator = ::System_Private_CoreLib::System::Collections::Generic::Dictionary_2_Enumerator<T0, T1>; } } }")]
[assembly: AssemblyEmbeddedCode("namespace System { namespace Collections { namespace Generic { template <class T0, class T1> using Dictionary_2_ValueCollection = ::System_Private_CoreLib::System::Collections::Generic::Dictionary_2_ValueCollection<T0, T1>; } } }")]