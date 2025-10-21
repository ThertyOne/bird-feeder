* MC34063 simplified behavioural macromodel
* Pin order (as in datasheet): 1 SW_COL, 2 SW_EMIT, 3 TIM, 4 GND, 5 COMP_INV(FB), 6 VCC, 7 IPK_SENSE, 8 DRIVE_COL
.subckt MC34063S 1 2 3 4 5 6 7 8 params: FREQ=50k TONMAX=4u IPEAK_TRIP=0.3 VREF=1.25

********************************************************************************
* NODES (internal):
* node_osc - oscillator pulse source output (set pulses)
* node_drive - base-drive node that actually drives internal switch transistor base
* node_sense - sense node (voltage across sense resistor)
* node_latch - SR latch output
********************************************************************************

* 1) Reference voltage (1.25 V)
VREF REF 0 DC {VREF}

* 2) Oscillator: simple pulse generator with frequency param FREQ and max on-time TONMAX.
*    The oscillator produces periodic set-pulses (node OSC). Period = 1/FREQ.
.param TPER = 1/{FREQ}
.param TON = {TONMAX}    ; max on-time per cycle (user can change)
VOSC OSC 0 PULSE(0 5 0 1u 1u {TON} {TPER})

* 3) Comparator for feedback: if FB voltage (pin 5) > VREF -> inhibit set pulses
*    We implement inhibition by gating OSC through an analog switch controlled by FB
.model SWFB SW(Ron=1 Roff=1Meg Vt={VREF} Vh=0.01)
SFB OSC_GATE OSC  FB 0 SWFB  ; control terminals FB-0 ; when V(FB)>VREF, switch closes and connects OSC to OSC_GATE
* Note: we invert logic: we want OSC_GATE = OSC only when FB < VREF. To do that
* we create a controlled inverter: use a small E source to invert FB relation
* Instead of complex logic, we'll use a piecewise expression later to create gate signal.
* For simplicity here we will use a behavioural source to pass OSC only when V(FB) < VREF
BOSC_GATE OSC_GATE 0 V=V(osc)* (V(5) < VREF ? 1 : 0)

* 4) SR-latch simplified: the OSC_GATE sets latch (turns switch on). Reset happens when:
*    - sense voltage at pin7 exceeds IPEAK_TRIP (current limit), OR
*    - FB (pin5) >= VREF (output high enough)
* We'll implement latch as a node that goes high on OSC_GATE pulse and is forced low by either condition.

* Node to model sense comparator - opens when sense > IPEAK_TRIP
.model SWSENSE SW(Ron=1 Roff=1Meg Vt={IPEAK_TRIP} Vh=0.01)
* SENSE comparator: if V(7)>Ipeaks -> force latch low. We'll implement as behavioural current sink pulling node_latch low.
BRESET LATCH 0 I = (V(7) > {IPEAK_TRIP}) ? 1e3 : 0

* SR latch behaviour (very simplified):
* When OSC_GATE goes high it charges C_LATCH via Rl, producing LATCH=HIGH.
R_LATCH OSC_GATE LATCH 1k
C_LATCH LATCH 0 1u
* Bleeder to ground to deassert slowly
R_LATCH_B LATCH 0 1Meg

* Reset by BRESET: if BRESET current >0 it will pull node LATCH down quickly
* emulate strong pull-down when triggered:
G_PULLDOWN LATCH 0 VALUE={ -1e3*(V(7) > {IPEAK_TRIP} || V(5) >= VREF) }

* 5) Drive transistor: use NPN switch (internal power transistor). Drive is a resistor from LATCH to transistor base.
QSW 8 2 0 QSWMOD
Rb_base LATCH 8 2k

* Model for internal transistor - moderate gain, approximate
.model QSWMOD NPN (Is=1e-14 BF=40 Vaf=50)
* Note: pin1 (SW_COL) is transistor collector (external node), pin2 is emitter

* 6) Current sense: external sense resistor should be connected between pin2 (SW_EMIT) and pin7 (IPK_SENSE),
*    or user can place sense resistor from pin2 to ground and feed pin7 accordingly. We simply expose pin7 for the user.

* 7) Connect VCC node: internal logic supply powers oscillator and reference (already implicit)

* 8) End subckt
.ends MC34063S
