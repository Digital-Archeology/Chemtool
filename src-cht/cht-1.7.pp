program cht;

type
		PointPtr=^TPoint;
		TPoint=record
			X:longint;
			Y:longint;
			C,H,N,O,P,S,Si,B,Br,Cl,F,I,Mg,Na,K:longint;
			RecCount:longint;
			Last:PointPtr;
		end;
		
		BondPtr=^TBond;
		TBond=record
			X1:longint;
			Y1:longint;
			X2:longint;
			Y2:longint;
			RecCount:longint;
			Last:BondPtr;
		end;
		
var C,H,N,O,P,S,Si,B,Br,Cl,F,I,Mg,Na,K,X1,X2,Y1,Y2,T:longint;
		IFile:text;
		Point,Last,Current:PointPtr;
		Bond,LastB,CurrentB:BondPtr;
		sss,A,subs,Alert:string;
		sssb:string[6];
		M,Me:real;
		separator:char;
		Count,code,atoms,rep,range,bonds:longint;
		Abandon,debug,verbose,fa,nobonds:boolean;
const	Sensi:longint=5; {bonds and labels joint precision sensitivity}

Procedure doBonds;
begin
	Repeat
		Readln(IFile,X1,Y1,X2,Y2,T);
		Dec(bonds);
		If debug then Writeln(X1,'; ',Y1,'; ',X2,'; ',Y2,'; ',T);
		If T=11 then range:=4*((X1-X2)*(X1-X2)+(Y1-Y2)*(Y1-Y2));
		
		LastB:=Bond;
		New(Bond);
		Bond^.X1:=X1;
		Bond^.X2:=X2;
		Bond^.Y1:=Y1;
		Bond^.Y2:=Y2;
		Bond^.Last:=LastB;
		CurrentB:=Bond;
		Bond:=Bond^.Last;
		While Bond^.Last<>nil do
			begin
				If (((ABS(Bond^.X1-X1)<Sensi) AND (ABS(Bond^.Y1-Y1)<Sensi) AND (ABS(Bond^.X2-X2)<Sensi) AND (ABS(Bond^.Y2-Y2)<Sensi)) OR
					((ABS(Bond^.X2-X1)<Sensi) AND (ABS(Bond^.Y2-Y1)<Sensi) AND (ABS(Bond^.X1-X2)<Sensi) AND (ABS(Bond^.Y1-Y2)<Sensi))) then
					begin
						If debug then Writeln('  ! Bond overlap');
						Alert:=Alert + '!';
					end;
				Bond:=Bond^.Last;
			end;
		Bond:=CurrentB;
		
		Abandon:=false;
		Current:=Point;
		While Point^.Last<>nil do
			begin
				If T=11 then
					begin
						Abandon:=true;
						If ((Point^.X-X1)*(Point^.X-X1)+(Point^.Y-Y1)*(Point^.Y-Y1)) < range then
							begin
								If Point^.H > 0 then Point^.H:=Point^.H-1;
								If debug then Writeln('Ring --> stripped one H from ',Point^.X,' ',Point^.Y);
							end;
					end
				else
					If ((ABS(Point^.X-X1)<Sensi) AND (ABS(Point^.Y-Y1)<Sensi)) then
						begin
							If debug then Writeln('  ',Point^.RecCount,' --> Joined at ',X1,' ',Y1);
							If (T=0) OR (T=5) OR (T=6) OR (T=7) OR (T=10) OR (T=12) OR (T=13) then Point^.H:=Point^.H-1;
							If (T=1) OR (T=2) OR (T=4) then Point^.H:=Point^.H-2;
							If (T=3) then Point^.H:=Point^.H-3;
							Abandon:=true;
						end;
				Point:=Point^.Last;
			end;
		Point:=Current;

		If NOT Abandon then
			begin
				Inc(Count);
				Last:=Point;
				New(Point);
				Point^.X:=X1;
				If debug then Write('  Point^.X=',Point^.X);
				Point^.Y:=Y1;
				If debug then Write('  Point^.Y=',Point^.Y);
				If T<>11 then Point^.C:=1 else Point^.C:=0;
				If debug then Write('  Point^.C=',Point^.C);
				If (T=0) OR (T=5) OR (T=6) OR (T=7) OR (T=10) OR (T=12) OR (T=13) then Point^.H:=3;
				If (T=1) OR (T=2) OR (T=4) then Point^.H:=2;
				If (T=3) then Point^.H:=1;
				If debug then Write('  Point^.H=',Point^.H);
				Point^.N:=0;
				Point^.O:=0;
				Point^.P:=0;
				Point^.S:=0;
				Point^.Si:=0;
				Point^.B:=0;
				Point^.Br:=0;
				Point^.Cl:=0;
				Point^.F:=0;
				Point^.I:=0;
				Point^.Na:=0;
				Point^.Mg:=0;
				Point^.K:=0;
				Point^.RecCount:=Count;
				If debug then Writeln('  Point^.RecCount=',Point^.RecCount);
				Point^.Last:=Last;
			end;

		If (T<>11) then
			begin
				Abandon:=false;
				Current:=Point;
				While Point^.Last<>nil do
					begin
						If ((ABS(Point^.X-X2)<Sensi) AND (ABS(Point^.Y-Y2)<Sensi)) then
							begin
								If debug then Writeln('  ',Point^.RecCount,' --> Joined at ',X2,' ',Y2);
								If (T=0) OR (T=5) OR (T=6) OR (T=7) OR (T=10) OR (T=12) OR (T=13) then Point^.H:=Point^.H-1;
								If (T=1) OR (T=2) OR (T=4) then Point^.H:=Point^.H-2;
								If (T=3) then Point^.H:=Point^.H-3;
								Abandon:=true;
							end;
						Point:=Point^.Last;
					end;
				Point:=Current;

				If NOT Abandon then
					begin
						Inc(Count);
						Last:=Point;
						New(Point);
						Point^.X:=X2;
						If debug then Write('  Point^.X=',Point^.X);
						Point^.Y:=Y2;
						If debug then Write('  Point^.Y=',Point^.Y);
						If T<>11 then Point^.C:=1 else Point^.C:=0;
						If debug then Write('  Point^.C=',Point^.C);
						If (T=0) OR (T=5) OR (T=6) OR (T=7) OR (T=10) OR (T=12) OR (T=13) then Point^.H:=3;
						If (T=1) OR (T=2) OR (T=4) then Point^.H:=2;
						If (T=3) then Point^.H:=1;
						If debug then Write('  Point^.H=',Point^.H);
						Point^.N:=0;
						Point^.O:=0;
						Point^.P:=0;
						Point^.S:=0;
						Point^.Si:=0;
						Point^.B:=0;
						Point^.Br:=0;
						Point^.Cl:=0;
						Point^.F:=0;
						Point^.I:=0;
						Point^.Mg:=0;
						Point^.Na:=0;
						Point^.K:=0;
						Point^.RecCount:=Count;
						If debug then Writeln('  Point^.RecCount=',Point^.RecCount);
						Point^.Last:=Last;
					end;
			end;
	Until (Bonds=0);
end;

Procedure doLabels;
var mul,gmul,gmpos,ggmul,ggmpos:longint;
		stripped:boolean;
begin
	If debug then Writeln('---- and now parse the labels ---------');
	If verbose then Writeln ('HeteroAtoms: ',atoms);
	For rep:= 1 to atoms do
		begin
			Readln(IFile,X1,Y1,separator,sss);
			A:=Copy(sss,1,Pos(#09,sss)-1);

			If debug then Writeln('Atom ',A, ' X=',X1,' Y=',Y1,' sss: ',sss);
			If nobonds then {initialize the Point structure if no bonds, only labels, defined}
				begin
					Inc(Count);
					Last:=Point;
					New(Point);
					Point^.X:=X1;
					Point^.Y:=Y1;
					Point^.C:=0;
					Point^.H:=0;
					Point^.N:=0;
					Point^.O:=0;
					Point^.P:=0;
					Point^.S:=0;
					Point^.Si:=0;
					Point^.B:=0;
					Point^.Br:=0;
					Point^.Cl:=0;
					Point^.F:=0;
					Point^.I:=0;
					Point^.Mg:=0;
					Point^.Na:=0;
					Point^.K:=0;
					Point^.RecCount:=Count;
					Point^.Last:=Last;
				end;
			Current:=Point;
			While Point^.Last<>nil do
				begin
					If ((ABS(Point^.X-X1)<Sensi) AND (ABS(Point^.Y-Y1)<Sensi)) then
						begin
							If debug then Writeln('  [',Point^.RecCount,']');
							Point^.C:=0;{we substitute -CH_x(-) by -R(-)}
							Point^.H:=0;
							Point^.N:=0;
							Point^.O:=0;
							Point^.P:=0;
							Point^.S:=0;
							Point^.Si:=0;
							Point^.B:=0;
							Point^.Br:=0;
							Point^.Cl:=0;
							Point^.F:=0;
							Point^.I:=0;
							Point^.Mg:=0;
							Point^.Na:=0;
							Point^.K:=0;
							mul:=1;
							gmul:=1;ggmul:=1;
							While A<>'' do
								begin
									stripped:=false;
									If Length(A) >= 5 then
										begin
											subs:=Copy(A,1,5);
											If Length(A)>6 then If A[6]='_' then Val(A[7],mul,code) else mul:=1;
											mul:=mul*gmul*ggmul;
											If subs='TBDMS' then
												begin {Tert-butyldimethylsilyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+6*mul;
													Point^.H:=Point^.H+15*mul;
													Point^.Si:=Point^.Si+1*mul;
													A:=Copy(A,6,Length(A)-5);
													stripped:=true;
												end;
											If subs='TBDPS' then
												begin {Tert-butyldiphenylsilyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+16*mul;
													Point^.H:=Point^.H+19*mul;
													Point^.Si:=Point^.Si+1*mul;
													A:=Copy(A,6,Length(A)-5);
													stripped:=true;
												end;
											If A[1]='[' then
												begin
													ggmpos:=Pos(']',A);
													If ggmpos <> 0 then If A[ggmpos+1] = '_' then Val(A[ggmpos+2],ggmul,code) else ggmul:=1;
												end;
											If A[1]='(' then
												begin
													gmpos:=Pos(')',A);
													If gmpos <> 0 then If A[gmpos+1] = '_' then Val(A[gmpos+2],gmul,code) else gmul:=1;
												end;
										end;
									If ((Length(A) >= 4) AND NOT stripped) then
										begin
											subs:=Copy(A,1,4);
											If Length(A)>5 then If A[5]='_' then Val(A[6],mul,code) else mul:=1;
											mul:=mul*gmul*ggmul;
											If subs='DBAM' then
												begin {dibutylaminomethylene; =R, used as the N prot.}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+9*mul;
													Point^.H:=Point^.H+19*mul;
													Point^.N:=Point^.N+1*mul;
													A:=Copy(A,5,Length(A)-4);
													stripped:=true;
												end;
											If subs='DMAM' then
												begin {dimethylaminomethylene; =R, used as the N prot.}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+3*mul;
													Point^.H:=Point^.H+7*mul;
													Point^.N:=Point^.N+1*mul;
													A:=Copy(A,5,Length(A)-4);
													stripped:=true;
												end;
											If subs='DMTr' then
												begin {dimethoxytrityl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+21*mul;
													Point^.H:=Point^.H+19*mul;
													Point^.O:=Point^.O+2*mul;
													A:=Copy(A,5,Length(A)-4);
													stripped:=true;
												end;
											If subs='MMTr' then
												begin {monomethoxytrityl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+20*mul;
													Point^.H:=Point^.H+16*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,5,Length(A)-4);
													stripped:=true;
												end;
											If subs='TMTr' then
												begin {trimethoxytrityl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+22*mul;
													Point^.H:=Point^.H+22*mul;
													Point^.O:=Point^.O+3*mul;
													A:=Copy(A,5,Length(A)-4);
													stripped:=true;
												end;
										end;
									If ((Length(A) >= 3) AND NOT stripped) then
										begin
											subs:=Copy(A,1,3);
											If Length(A)>4 then If A[4]='_' then Val(A[5],mul,code) else mul:=1;
											mul:=mul*gmul*ggmul;
											If subs='Ade' then
												begin {Adeninyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+5*mul;
													Point^.H:=Point^.H+4*mul;
													Point^.N:=Point^.N+5*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='BOC' then
												begin {Butyloxycarbonyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+5*mul;
													Point^.H:=Point^.H+9*mul;
													Point^.O:=Point^.O+2*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='Cyt' then
												begin {Cytosinyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+4*mul;
													Point^.H:=Point^.H+4*mul;
													Point^.N:=Point^.N+3*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='Gua' then
												begin {Guaninyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+5*mul;
													Point^.H:=Point^.H+4*mul;
													Point^.N:=Point^.N+5*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='iBu' then
												begin {iso-Butyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+4*mul;
													Point^.H:=Point^.H+9*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='iPr' then
												begin {2-Propyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+3*mul;
													Point^.H:=Point^.H+7*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='MOC' then
												begin {Methoxycarbonyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+2*mul;
													Point^.H:=Point^.H+3*mul;
													Point^.O:=Point^.O+2*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='MOM' then
												begin {Methoxymethyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+2*mul;
													Point^.H:=Point^.H+5*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='tBu' then
												begin {t-Butyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+4*mul;
													Point^.H:=Point^.H+9*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='Thy' then
												begin {Thyminyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+5*mul;
													Point^.H:=Point^.H+5*mul;
													Point^.N:=Point^.N+2*mul;
													Point^.O:=Point^.O+2*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='TMS' then
												begin {Trimethylsilyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+3*mul;
													Point^.H:=Point^.H+9*mul;
													Point^.Si:=Point^.Si+1*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='Tol' then
												begin {tolyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+8*mul;
													Point^.H:=Point^.H+7*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
											If subs='Ura' then
												begin {Uracilyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+4*mul;
													Point^.H:=Point^.H+3*mul;
													Point^.N:=Point^.N+2*mul;
													Point^.O:=Point^.O+2*mul;
													A:=Copy(A,4,Length(A)-3);
													stripped:=true;
												end;
										end;
									If ((Length(A) >= 2) AND NOT stripped) then
										begin
											subs:=Copy(A,1,2);
											If Length(A)>3 then If A[3]='_' then Val(A[4],mul,code) else mul:=1;
											mul:=mul*gmul*ggmul;
											If subs='Ac' then
												begin {acetyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+2*mul;
													Point^.H:=Point^.H+3*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Bn' then
												begin {benzyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+7*mul;
													Point^.H:=Point^.H+7*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Br' then
												begin {bromine}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.Br:=Point^.Br+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Bu' then
												begin {butyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+4*mul;
													Point^.H:=Point^.H+9*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Bz' then
												begin {benzoyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+7*mul;
													Point^.H:=Point^.H+5*mul;
													Point^.O:=Point^.O+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='CE' then
												begin {cyanoethyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+3*mul;
													Point^.H:=Point^.H+4*mul;
													Point^.N:=Point^.N+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Cl' then
												begin {chlorine}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.Cl:=Point^.Cl+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Et' then
												begin {ethyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+2*mul;
													Point^.H:=Point^.H+5*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Me' then
												begin {methyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+1*mul;
													Point^.H:=Point^.H+3*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Ms' then
												begin {methanesulfonyl, mesyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+1*mul;
													Point^.H:=Point^.H+3*mul;
													Point^.O:=Point^.O+2*mul;
													Point^.S:=Point^.S+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Mg' then
												begin {magnesium}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.Mg:=Point^.Mg+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Na' then
												begin {sodium}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.Na:=Point^.Na+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Ph' then
												begin {phenyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+6*mul;
													Point^.H:=Point^.H+5*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Pr' then
												begin {propyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+3*mul;
													Point^.H:=Point^.H+7*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Tf' then
												begin {trifluoromethanesulfonyl, triflyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+1*mul;
													Point^.F:=Point^.F+3*mul;
													Point^.O:=Point^.O+2*mul;
													Point^.S:=Point^.S+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Tr' then
												begin {trityl, triphenylmethyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+19*mul;
													Point^.H:=Point^.H+15*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Ts' then
												begin {toluenesulfonyl, tosyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+7*mul;
													Point^.H:=Point^.H+7*mul;
													Point^.O:=Point^.O+2*mul;
													Point^.S:=Point^.S+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
											If subs='Si' then
												begin {silicon}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.Si:=Point^.Si+1*mul;
													A:=Copy(A,3,Length(A)-2);
													stripped:=true;
												end;
										end;
									If ((Length(A) >= 1) AND NOT stripped) then
										begin
											subs:=Copy(A,1,1);
											If Length(A)>2 then If A[2]='_' then Val(A[3],mul,code) else mul:=1;
											mul:=mul*gmul*ggmul;
											If subs='B' then
												begin {boron}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.B:=Point^.B+1*mul;
												end;
											If subs='C' then
												begin {carbon}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+1*mul;
												end;
											If subs='F' then
												begin {fluorine}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.F:=Point^.F+1*mul;
												end;
											If subs='H' then
												begin {hydrogen}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.H:=Point^.H+1*mul;
												end;
											If subs='I' then
												begin {iodine}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.I:=Point^.I+1*mul;
												end;
											If subs='K' then
												begin {potassium}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.K:=Point^.K+1*mul;
												end;
											If subs='N' then
												begin {nitrogen}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.N:=Point^.N+1*mul;
												end;
											If subs='O' then
												begin {oxygen}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.O:=Point^.O+1*mul;
												end;
											If subs='P' then
												begin {phosphorus}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.P:=Point^.P+1*mul;
												end;
											If subs='S' then
												begin {sulfur}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.S:=Point^.S+1*mul;
												end;
											If subs='Z' then
												begin {"Z", Benzyloxycarbonyl}
													If debug then Writeln('  ',subs,'*',mul,' (',gmul,',',ggmul,')');
													Point^.C:=Point^.C+8*mul;
													Point^.H:=Point^.H+7*mul;
													Point^.O:=Point^.O+2*mul;
												end;
											If subs=')' then
												begin {reset gmul}
													gmul:=1;
												end;
											If subs=']' then
												begin {reset gmul,ggmul}
													gmul:=1;
													ggmul:=1;
												end;
											If NOT stripped then A:=Copy(A,2,Length(A)-1);
										end;
								end;
					end;
				Point:=Point^.Last;
			end;
		Point:=Current;
	end;
end;


begin
	If (Paramcount=0) then
		begin
			Writeln('enter "cht -h" for help');
			Halt(0);
		end;
	C:=0;H:=0;N:=0;O:=0;P:=0;S:=0;Si:=0;B:=0;Br:=0;I:=0;Cl:=0;F:=0;Mg:=0;Na:=0;K:=0;
	verbose:=false;
	debug:=false;
	fa:=false;{no file assigned}
	Count:=0;
	Alert:='';
	For rep:= 1 to Paramcount do
		begin
			If (Paramstr(rep)='-v') OR (Paramstr(rep)='--verbose') then verbose:=true;
			If (Paramstr(rep)='-d') OR (Paramstr(rep)='--debug') then debug:=true;
			If (Paramstr(rep)='-h') OR (Paramstr(rep)='--help') then
				begin
					Writeln('The Chemtool drawings analyzer 1.7');
					Writeln('***   Gizmo Head Software   ***');
					Writeln('    Radek Liboska (c) 2001');
					Writeln;
					Writeln('syntax: cht [-options] <filename.cht>');
					Writeln;
					Writeln('Recognizes C,H,O,N,P,S,Si,B,Br,I,Cl,F,Mg,Na,K,');
					Writeln('        Ac,Ade,Bn,Bu,Bz,BOC,Cyt,CE,DBAM,DMAM,DMTr,Et,Gua,iBu,iPr,Me,Ms,MOC,');
					Writeln('        MOM,MMTr,Ph,Pr,tBu,Tf,Thy,Tol,Tr,Ts,TBDMS,TBDPS,TMS,TMTr,Ura,Z');
					Writeln('Can handle two levels of patentheses; e.g. P[OCH(CH_3)_2]_3');
					Writeln;
					Writeln('options:');
					Writeln('        -h or --help : this help');
					Writeln('        -v or --verbose : be verbose');
					Writeln('        -d or --debug : be more verbose');
					Writeln;
					Halt(0);
				end;
			If Paramstr(rep)[1]<>'-' then
				begin
					Assign(IFile,Paramstr(rep));
					fa:=true;
				end;
		end;
	If NOT fa then
		begin
			Writeln('No file defined, enter "cht -h" for help');
			Halt(1);
		end;
	Reset(IFile);
	New(Point);
	Point^.X:=0;
	Point^.Y:=0;
	Point^.C:=0;
	Point^.H:=0;
	Point^.N:=0;
	Point^.O:=0;
	Point^.P:=0;
	Point^.S:=0;
	Point^.Si:=0;
	Point^.B:=0;
	Point^.Br:=0;
	Point^.Cl:=0;
	Point^.F:=0;
	Point^.I:=0;
	Point^.Mg:=0;
	Point^.Na:=0;
	Point^.K:=0;
	Point^.RecCount:=Count;
	Point^.Last:=nil;
	New(Bond);
	Bond^.X1:=0;
	Bond^.Y1:=0;
	Bond^.X2:=0;
	Bond^.Y2:=0;
	Bond^.RecCount:=Count;
	Bond^.Last:=nil;
	Readln(IFile,sss);
	sss:=Copy(sss,1,18);
	If NOT (sss='Chemtool Version 1') then
		begin
			Writeln('Not a chemtool 1.x file, enter "cht -h" for help');
			Halt(2);
		end;
	Readln(IFile);
	Readln(IFile,sssb,bonds);
	If bonds=0 then nobonds:=true;
	If sssb='bonds ' then If bonds > 0 then doBonds;
	Readln(IFile,sssb,atoms);
	If sssb='atoms ' then If atoms > 0 then doLabels;
	Close(IFile);
	If verbose then Writeln('---------------------- Summary -------------------------');
	While Point^.Last<>nil do
		begin
			If verbose then 
				Writeln('RecCount: ',Point^.RecCount,' X=',Point^.X,' Y=',Point^.Y,' C',Point^.C,' H',Point^.H,' N',Point^.N,' O',Point^.O,' P',Point^.P,' S',Point^.S,' Si',Point^.Si,' B',Point^.B,' Br',Point^.Br,' Cl',Point^.Cl,' F',Point^.F,' I',Point^.I,' Mg',Point^.Mg,' Na',Point^.Na,' K',Point^.K);
			C:=C+Point^.C;
			H:=H+Point^.H;
			N:=N+Point^.N;
			O:=O+Point^.O;
			P:=P+Point^.P;
			S:=S+Point^.S;
			Si:=Si+Point^.Si;
			B:=B+Point^.B;
			Br:=Br+Point^.Br;
			Cl:=Cl+Point^.Cl;
			F:=F+Point^.F;
			I:=I+Point^.I;
			Mg:=Mg+Point^.Mg;
			Na:=Na+Point^.Na;
			K:=K+Point^.K;

			Point:=Point^.Last;
		end;

	If H<0 then H:=0; {overlapped bonds}
	
	M:=C*12.011+H*1.0079+N*14.0067+O*15.9994+P*30.97376+S*32.064+Si*28.086+B*10.81+Br*79.904+Cl*35.453+F*18.9984+I*126.9045+Mg*24.305+Na*22.98977+K*39.098;
	Me:=C*12+H*1.007825037+N*14.003074008+O*15.99491464+P*30.9737634+S*31.9720718+Si*27.9769284+B*11.0093053+Br*78.9183361+Cl*34.968852729+F*18.99840325+I*126.904477+Mg*23.9850450+Na*22.9897697+K*38.9637079;

	If ((Me>0) AND (Alert='')) then
		begin
			If C>1 then Write('C',C);If C=1 then Write('C');
			If H>1 then Write('H',H);If H=1 then Write('H');
			If N>1 then Write('N',N);If N=1 then Write('N');
			If O>1 then Write('O',O);If O=1 then Write('O');
			If P>1 then Write('P',P);If P=1 then Write('P');
			If S>1 then Write('S',S);If S=1 then Write('S');
			If Si>1 then Write('Si',Si);If Si=1 then Write('Si');
			If B>1 then Write('B',B);If B=1 then Write('B');
			If Br>1 then Write('Br',Br);If Br=1 then Write('Br');
			If Cl>1 then Write('Cl',Cl);If Cl=1 then Write('Cl');
			If F>1 then Write('F',F);If F=1 then Write('F');
			If I>1 then Write('I',I);If I=1 then Write('I');
			If Mg>1 then Write('Mg',Mg);If Mg=1 then Write('Mg');
			If Na>1 then Write('Na',Na);If Na=1 then Write('Na');
			If K>1 then Write('K',K);If K=1 then Write('K');

			Write(' [',M:2:3,']');
			Write(' Me=',Me:2:10,' ');
			
			If C > 0 then Write(C*12.011*100/M:4:2,'%C;');
			If H > 0 then Write(H*1.0079*100/M:4:2,'%H;');
			If B > 0 then Write(B*10.81*100/M:4:2,'%B;');
			If N > 0 then Write(N*14.0067*100/M:4:2,'%N;');
			If O > 0 then Write(O*15.9994*100/M:4:2,'%O;');
			If P > 0 then Write(P*30.97376*100/M:4:2,'%P;');
			If S > 0 then Write(S*32.0640*100/M:4:2,'%S;');
			If K > 0 then Write(K*39.098*100/M:4:2,'%K;');
			If Mg > 0 then Write(Mg*24.305*100/M:4:2,'%Mg;');
			If Na > 0 then Write(Na*22.98977*100/M:4:2,'%Na;');
			If Si > 0 then Write(Si*28.086*100/M:4:2,'%Si;');
			If Br > 0 then Write(Br*79.904*100/M:4:2,'%Br;');
			If Cl > 0 then Write(Cl*35.453*100/M:4:2,'%Cl;');
			If F > 0 then Write(F*18.9984*100/M:4:2,'%F;');
			If I > 0 then Write(I*126.9045*100/M:4:2,'%I;');
		end
	else
		If Alert<>'' then Write(Alert,' overlapped bonds !') 
			else Write('C0H0 [0.00] Me=0.0000000000 0.00%C;0.00%H');
	Writeln;
end.
