/**
 *	\mainpage Alitronika Digital Video ATDemoApp
 *
	\htmlonly
	<A HREF="http://www.alitronika.com">
	\endhtmlonly
		<img border="0" src="allogobar.gif">
	\htmlonly
	</A>
	\endhtmlonly
 *
 *	\par Introduction
 *	For development of an interface into customer software, the API is complemented
 *	with a demo program 'ATDemoApp'. This program is available in binary and source code,
 *	within the ATDV_API installation directory.\n
 *	This program may be used as source code example on how to control devices from your application.
 *	The modules ATBoardPlayRec.cpp, ATBoardTunDVBT.cpp etc. may even be used directly in a custom
 *	applications.\n
 *	
 *	\par Calling layering structure
 *	\dot
 digraph G
 {
	ATDemoApp [shape=box,   color=blue,style=filled,fillcolor=azure,fontsize=9,width=2,label="Alitronika provided\nATDemoApp Program"];
	Api [shape=record,color=blue,style=filled,fillcolor=azure,fontsize=9,width=3,label="{<f0> ATDV API |{<f1> Registers |<f2> DVB-Sources\n(AT678/17/18/27/28 only) }}"];
	Drv [shape=record,color=blue,style=filled,fillcolor=azure,fontsize=9,width=3,label="{<f0> Driver |{<f1> USB |<f2> PCI }}"];
	Brd [shape=record,color=blue,style=filled,fillcolor=azure,fontsize=9,width=3,label="Alitronika USB or PCI device(s)"];
	ATDemoApp -> Api;
	Api:f1-> Drv;
	Api:f2-> Drv;
	Drv -> Brd;
 }
\enddot
 *
 *	\par
 *	\htmlonly <hr><font size="-1"><I> \endhtmlonly
 *	With respect to documents provided with this API, neither Alitronika nor any of their suppliers or employees,
 *	makes any warranty, express or implied, including the warranties of merchantability and fitness
 *	for a particular purpose, or assumes any legal liability or responsibility for the accuracy,
 *	completeness, or usefulness of any information, software, apparatus, product, or process disclosed,
 *	or represents that its use would not infringe privately owned rights.
 *	\par
 *	Any reproduction or copying, whether partial or entire, of any material contained in this document or
 *	its related files is strictly prohibited without the express prior permission of Alitronika.
 *	\htmlonly </I></font> \endhtmlonly
 *
 */
