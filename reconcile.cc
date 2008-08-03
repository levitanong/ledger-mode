#include "reconcile.h"
#include "walk.h"

namespace ledger {

#define xact_next(x)     reinterpret_cast<xact_t *>(x->xdata().ptr)
#define xact_next_ptr(x) reinterpret_cast<xact_t **>(&x->xdata().ptr)

static bool search_for_balance(amount_t& amount,
			       xact_t ** prev, xact_t * next)
{
  for (; next; next = xact_next(next)) {
    xact_t * temp = *prev;
    *prev = next;

    amount -= next->amount;
    if (! amount ||
	search_for_balance(amount, xact_next_ptr(next), xact_next(next)))
      return true;
    amount += next->amount;

    *prev = temp;
  }
  return false;
}

void reconcile_xacts::push_to_handler(xact_t * first)
{
  for (; first; first = xact_next(first))
    item_handler<xact_t>::operator()(*first);

  item_handler<xact_t>::flush();
}

void reconcile_xacts::flush()
{
  value_t cleared_balance;
  value_t pending_balance;

  xact_t *  first    = NULL;
  xact_t ** last_ptr = &first;

  foreach (xact_t * xact, xacts) {
    if (! is_valid(cutoff) || xact->date() < cutoff) {
      switch (xact->state) {
      case xact_t::CLEARED:
	cleared_balance += xact->amount;
	break;
      case xact_t::UNCLEARED:
      case xact_t::PENDING:
	pending_balance += xact->amount;
	*last_ptr = xact;
	last_ptr = xact_next_ptr(xact);
	break;
      }
    }
  }

  if (cleared_balance.type() >= value_t::BALANCE)
    throw std::runtime_error("Cannot reconcile accounts with multiple commodities");

  cleared_balance.cast(value_t::AMOUNT);
  balance.cast(value_t::AMOUNT);

  commodity_t& cb_comm = cleared_balance.as_amount().commodity();
  commodity_t& b_comm  = balance.as_amount().commodity();

  balance -= cleared_balance;
  if (balance.type() >= value_t::BALANCE)
    throw_(std::runtime_error,
	   "Reconcile balance is not of the same commodity ('"
	   << b_comm.symbol() << "' != '" << cb_comm.symbol() << "')");

  // If the amount to reconcile is the same as the pending balance,
  // then assume an exact match and return the results right away.
  amount_t& to_reconcile(balance.as_amount_lval());
  pending_balance.cast(value_t::AMOUNT);
  if (to_reconcile == pending_balance.as_amount() ||
      search_for_balance(to_reconcile, &first, first)) {
    push_to_handler(first);
  } else {
    throw std::runtime_error("Could not reconcile account!");
  }
}

} // namespace ledger
