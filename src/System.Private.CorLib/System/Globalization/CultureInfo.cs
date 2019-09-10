using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;

namespace System.Globalization
{
    public class CultureInfo: IFormatProvider
    {
        //--------------------------------------------------------------------//
        //                        Internal Information                        //
        //--------------------------------------------------------------------//

        // We use an RFC4646 type string to construct CultureInfo.
        // This string is stored in _name and is authoritative.
        // We use the _cultureData to get the data for our object

        private bool _isReadOnly;
        private CompareInfo compareInfo;
        internal NumberFormatInfo numInfo;
        //
        // The CultureData instance that we are going to read data from.
        // For supported culture, this will be the CultureData instance that read data from mscorlib assembly.
        // For customized culture, this will be the CultureData instance that read data from user customized culture binary file.
        //
        internal CultureData _cultureData;

        internal bool _isInherited;

        private CultureInfo _consoleFallbackCulture;

        // Names are confusing.  Here are 3 names we have:
        //
        //  new CultureInfo()   _name          _nonSortName    _sortName
        //      en-US           en-US           en-US           en-US
        //      de-de_phoneb    de-DE_phoneb    de-DE           de-DE_phoneb
        //      fj-fj (custom)  fj-FJ           fj-FJ           en-US (if specified sort is en-US)
        //      en              en              
        //
        // Note that in Silverlight we ask the OS for the text and sort behavior, so the 
        // textinfo and compareinfo names are the same as the name

        // This has a de-DE, de-DE_phoneb or fj-FJ style name
        internal string _name;

        // This will hold the non sorting name to be returned from CultureInfo.Name property.
        // This has a de-DE style name even for de-DE_phoneb type cultures
        private string _nonSortName;

        // This will hold the sorting name to be returned from CultureInfo.SortName property.
        // This might be completely unrelated to the culture name if a custom culture.  Ie en-US for fj-FJ.
        // Otherwise its the sort name, ie: de-DE or de-DE_phoneb
        private string _sortName;

        //The Invariant culture;
        private static readonly CultureInfo s_InvariantCultureInfo = new CultureInfo(CultureData.Invariant, isReadOnly: true);

        // LOCALE constants of interest to us internally and privately for LCID functions
        // (ie: avoid using these and use names if possible)
        internal const int LOCALE_NEUTRAL = 0x0000;
        private const int LOCALE_USER_DEFAULT = 0x0400;
        private const int LOCALE_SYSTEM_DEFAULT = 0x0800;
        internal const int LOCALE_CUSTOM_UNSPECIFIED = 0x1000;
        internal const int LOCALE_CUSTOM_DEFAULT = 0x0c00;
        internal const int LOCALE_INVARIANT = 0x007F;

        ////////////////////////////////////////////////////////////////////////
        //
        //  InvariantCulture
        //
        //  This instance provides methods, for example for casing and sorting,
        //  that are independent of the system and current user settings.  It
        //  should be used only by processes such as some system services that
        //  require such invariant results (eg. file systems).  In general,
        //  the results are not linguistically correct and do not match any
        //  culture info.
        //
        ////////////////////////////////////////////////////////////////////////

        private CultureInfo(CultureData cultureData, bool isReadOnly = false)
        {
            Debug.Assert(cultureData != null);
            _cultureData = cultureData;
            _name = cultureData.CultureName;
            _isInherited = false;
            _isReadOnly = isReadOnly;
        }

        public static CultureInfo CurrentCulture => InvariantCulture;

        public static CultureInfo InvariantCulture
        {
            get
            {
                Debug.Assert(s_InvariantCultureInfo != null);
                return (s_InvariantCultureInfo);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        //
        //  CompareInfo               Read-Only Property
        //
        //  Gets the CompareInfo for this culture.
        //
        ////////////////////////////////////////////////////////////////////////
        public virtual CompareInfo CompareInfo
        {
            get
            {
                if (this.compareInfo == null)
                {
                    // Since CompareInfo's don't have any overrideable properties, get the CompareInfo from
                    // the Non-Overridden CultureInfo so that we only create one CompareInfo per culture
                    this.compareInfo = new CompareInfo(this);
                }
                return (compareInfo);
            }
        }

        public virtual NumberFormatInfo NumberFormat
        {
            get
            {
                if (numInfo == null)
                {
                    NumberFormatInfo temp = new NumberFormatInfo(_cultureData);
                    temp.isReadOnly = _isReadOnly;
                    Interlocked.CompareExchange(ref numInfo, temp, null);
                }
                return (numInfo);
            }
            set
            {
                if (value == null)
                {
                    throw new ArgumentNullException(nameof(value), SR.ArgumentNull_Obj);
                }
                VerifyWritable();
                numInfo = value;
            }
        }

        private void VerifyWritable()
        {
            if (_isReadOnly)
            {
                throw new InvalidOperationException(SR.InvalidOperation_ReadOnly);
            }
        }


        public virtual Object GetFormat(Type formatType)
        {
            if (formatType == typeof(NumberFormatInfo))
                return (NumberFormat);
            return (null);
        }
    }
}
