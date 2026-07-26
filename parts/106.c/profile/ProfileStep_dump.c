
void ProfileStep::dump() const
{
  LOG(". this=%p"
      " step_ms=%u"
      " I_A=%i"
      " sub_step_ms=%u"
      " PS_V_max=%u"
      " cBat_max_mV=%u"
      " cBat_min_mV=%u"
      " aBat_max_mV=%u"
      " aBat_min_mV=%u"
      " cBat_T_max_C=%u"
      " aBat_T_max_C=%u",
      this,
      step_ms,
      I_A,
      sub_step_ms,
      PS_V_max,
      cBat_max_mV,
      cBat_min_mV,
      aBat_max_mV,
      aBat_min_mV,
      cBat_T_max_C,
      aBat_T_max_C);
}
