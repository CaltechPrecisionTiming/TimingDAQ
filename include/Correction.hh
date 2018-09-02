double Correction(unsigned iCh, double tot)
{
   switch(iCh) {
   case 1  :
      return (0.91826*tot - 0.700535)*tot - 1.70613 + 1.84195;
      
   case 2  :
      if(tot < 0.46) return 0.;
	  return (1.14091*tot - 0.898552)*tot - 1.67092 + 1.84195;
  
   default : 
      return 0.;
}
}