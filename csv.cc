#include "csv.h"

namespace ledger {

namespace {
  inline void write_escaped_string(std::ostream& out, const string& xact)
  {
    out << "\"";
    foreach (char ch, xact)
      if (ch == '"') {
	out << "\\";
	out << "\"";
      } else {
	out << ch;
      }
    out << "\"";
  }
}

void format_csv_xacts::operator()(xact_t& xact)
{
  if (! xact.has_xdata() ||
      ! xact.xdata().has_flags(XACT_EXT_DISPLAYED)) {
    {
      format_t fmt("%D");
      std::ostringstream str;
#if 0
      fmt.format(str, details_t(xact));
#endif
      write_escaped_string(out, str.str());
    }
    out << ',';

    {
      format_t fmt("%P");
      std::ostringstream str;
#if 0
      fmt.format(str, details_t(xact));
#endif
      write_escaped_string(out, str.str());
    }
    out << ',';

    {
      format_t fmt("%A");
      std::ostringstream str;
#if 0
      fmt.format(str, details_t(xact));
#endif
      write_escaped_string(out, str.str());
    }
    out << ',';

    {
      format_t fmt("%t");
      std::ostringstream str;
#if 0
      fmt.format(str, details_t(xact));
#endif
      write_escaped_string(out, str.str());
    }
    out << ',';

    {
      format_t fmt("%T");
      std::ostringstream str;
#if 0
      fmt.format(str, details_t(xact));
#endif
      write_escaped_string(out, str.str());
    }
    out << ',';

    switch (xact.state) {
    case xact_t::CLEARED:
      write_escaped_string(out, "*");
      break;
    case xact_t::PENDING:
      write_escaped_string(out, "!");
      break;
    default: {
      xact_t::state_t state;
      if (xact.entry->get_state(&state))
	switch (state) {
	case xact_t::CLEARED:
	  write_escaped_string(out, "*");
	  break;
	case xact_t::PENDING:
	  write_escaped_string(out, "!");
	  break;
	default:
	  write_escaped_string(out, "");
	  break;
	}
    }
    }
    out << ',';

    if (xact.entry->code)
      write_escaped_string(out, *xact.entry->code);
    out << ',';

    {
      format_t fmt("%N");
      std::ostringstream str;
#if 0
      fmt.format(str, details_t(xact));
#endif
      write_escaped_string(out, str.str());
    }
    out << '\n';

    xact.xdata().add_flags(XACT_EXT_DISPLAYED);
  }
}

} // namespace ledger
