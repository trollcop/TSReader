#!/usr/bin/perl

#********************************************************
#* Some code. Copyright (C) 2003 by Pascal Massimino.   *
#* All Rights Reserved.      (http://skal.planet-d.net) *
#* For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
#********************************************************
######################################################
#
#  Makefile generator (=>./makefile.all)
#
# This is a "script-velu-qui-fait-tout" (tm)
#
# Not really finished (especially .dsp and .so stuff)
#
######################################################

$root_dir = "..";
$src_dir = "$root_dir/src";
$inc_dir = "$root_dir/include";

$config_file = "./PROJECT";
$makefile = "./makefile.all";
$doctxt = "../doc/files.txt";

@Ex_Bin;
@Ex_Src;
@Ex_Dep;
@Ex_Flag;
@Ex_Type;

@Shared_Bin;
@Shared_Bin_So;
@Shared_Src;
@Shared_Flag;
@Shared_Link;

%Spc_Flags;
@Libs_Flags;
@Libs_so_link;

%Libs_src_dirs;

#########################################
##         read config file            ##
#########################################

sub parse_example
{
  my $elm;

  push(@Ex_Bin, @_[0]);
  

  foreach $elm (split(' ', @_[1])) { push(@Ex_Src, $elm); } 
  push( @Ex_Src,";");

  foreach $elm (split(' ', @_[2])) { push(@Ex_Dep, $elm); }
  push( @Ex_Dep,";");

  if (@_[3]) {
    foreach $elm (split(' ', @_[3])) { push( @Ex_Flag, "\$(DEF)$elm" ); }
    push( @Ex_Flag,";");
  }

  push(@Ex_Type, @_[4]);
}

sub parse_shared  # $name, $src, $flag, $deps
{
  my @a = split(' ', @_[0]);
  push(@Shared_Bin, @a[0]);
  push(@Shared_Bin_So, "@a[0].so");

  my $elm;
  foreach $elm (split(' ', @_[1])) { push(@Shared_Src, $elm); }
  push( @Shared_Src,";");

  foreach $elm (split(' ', @_[2])) { push(@Shared_Flag, "\$(DEF)$elm"); }
  push( @Shared_Flag,";");

  foreach $elm (split(' ', @_[3])) { push(@Shared_Link, "$elm"); }
  push( @Shared_Link,";");
}

#########################################

sub read_config_file 
{
  my $line;
  open(FILES,"$config_file") || die( "Can't open $config_file file.\n");
  while($line=<FILES>)
  {
    chomp $line;
    my @lines = split( '#', $line);
    $line = $lines[0];
    if (!$line) { next; }
    if (!split(' ', $line)) { next; }

    if ($line =~ /^\s*example\s+(\w+)\s*:\s*(\w*)\s*:\s*([\w\s]*)\s*:\s*(\w*)/) {
#      print "example = [$1][$2][$3][$4]\n";
      parse_example($1, $2, $3, $4, 0);
    }
    elsif ($line =~ /^\s*util\s+(\w+)\s*:\s*([\w\s]*)\s*:\s*([\w\s]*)\s*:\s*(\w*)/) {
#      print "util = [$1][$2][$3][$4]\n";
      parse_example($1, $2, $3, $4, 1);
    }
    elsif ($line =~ /^\s*shared\s+(\w+)\s*:\s*([\w\s\/\.]+)\s*:\s*(\w*)\s*:\s*([\w\s]*)/) {
      parse_shared($1, $2, $3, $4);
    }
    elsif ($line =~ /^\s*exclude\s+([\w\.]+)\s*/) {
      @Line = split( ' ', $1);
      @Exclude{@Line} = 1;
    }
    elsif ($line =~ /^\s*conditional\s+(\w+)\s*/) {
      @Line = split( ' ', $1);
      my $obj = @Line[0]; 
      @Conditional_Flag{$obj} = @Line[1];
      @Conditional_Value{$obj} = @Line[2];
    }
    elsif ($line =~ /^\s*include\s+(\w+)\s*/) {
      @Line = split( ' ', $1);
      push(@include_dir,"$root_dir/@Line[0]");
    }
    elsif ($line =~ /^\s*flag\s+([\w\.]+)\s+(\w*)/) {
      my $src = $1;
#      print "special flag '$2' for source '$src'.\n";
      my $elm;
      foreach $elm (split( ' ',$2)) { @Spc_Flags{$src} .= "\$(DEF)$elm "; }
    }
    elsif ($line =~ /^\s*lib\s+(\w+)\s*:\s*(\w+)\s*:\s*([\w\s\/,\(\)\$]+)\s*:\s*(\w*)\s*:\s*([\w\s]*)/) {
#      print "lib = [$1][$2][$3][$4][$5]\n";
      my @Line;

      @Line = split( ' ', $1);
      my $lib = $Line[0];
      push(@libs, $lib);

      @Line = split( ' ', $2);
      @libs_label{$lib} = $Line[0];

      @Line = split( ', ', $3 );
      $line = join( ",", @Line );
      my @List = split( ' ', $line );
      @Libs_src_dirs{$lib} = $List[0];

      while(@Line) {
        my @idir = split( ' ', @Line[0]);
        $idir[0] = "$src_dir/$idir[0]";
        push(@include_dir, $idir[0]);
        shift @Line;
      }

      if ($4) {
        @Line = split( ' ', $4);
        @Libs_Flags{$lib} = $Line[0];
      }
      if ($5) {
        @Libs_so_link{$lib} = $5;
      }
    }
    else {
      die( "unrecognized entry directive '$line'\n" );
    }
  }
  close FILES;
}

#########################################
##      generation of makefile         ##
#########################################

sub write_cpp_entry
{
  my $path = @_[0]; shift @_;
  my $src = @_[0]; shift @_;
  my $cpp = "$path$src.\$(CPP)";
  my $obj = "\$(OBJ_OUTPUT_DIR)$src@_[0].\$(O)"; shift @_;
  print OUT "$obj: $cpp\n";
  print OUT "\t\$(CC) \$(CC_FLAGS) $cpp \$(OUT) $obj";
  while(@_[0]) { print OUT " @_[0]"; shift @_; }
  my $name = "$src.cpp";
  if (@Spc_Flags{$name}) { print OUT " @Spc_Flags{$name}"; }
  print OUT "\n\n";
}

sub write_asm_entry
{
  my $path = @_[0]; shift @_;
  my $src = @_[0]; shift @_;
  my $asm = "$path$src.\$(ASM)";
  my $obj = "\$(OBJ_OUTPUT_DIR)$src.\$(O)"; shift @_;
  print OUT "$obj: $asm\n";
  print OUT "\t\$(NASM) \$(NASM_FLAGS) $asm -o $obj\n";
  print OUT "\n\n";
}

#########################################

sub write_cpp_so_entry
{
  my $src = @_[0]; shift @_;
  my @toto = split('/', $src); my $src2 = @toto[$toto-1];
  my $cpp = "\$(LIB_SRC_DIR)$src.\$(CPP)";
  my $obj = "\$(LIB_SO_OUTPUT_DIR)$src2@_[0].\$(O)"; shift @_;
  print OUT "$obj: $cpp\n";
  print OUT "\t\$(CC) \$(CC_FLAGS) $cpp \$(OUT) $obj";
  while(@_[0]) { print OUT " @_[0]"; shift @_; }
  my $name = "$src2.cpp";
  if (@Spc_Flags{$name}) { print OUT " @Spc_Flags{$name}"; }
  print OUT "\n\n";
}

sub write_asm_so_entry
{
  my $src = @_[0]; shift @_;
  my @toto = split('/', $src); my $src2 = @toto[$toto-1];
  my $asm = "\$(LIB_SRC_DIR)$src.\$(ASM)";
  my $obj = "\$(LIB_SO_OUTPUT_DIR)$src2@_[0].\$(O)"; shift @_;
  print OUT "$obj: $asm\n";
  print OUT "\t\$(NASM) \$(NASM_FLAGS) $asm \$(OUT) $obj\n";
#  while(@_) { print OUT " @_[0]"; shift @_; }
  print OUT "\n\n";
}

sub write_so_entry  # $so_name, @Src, ";", @Links
{  
  my $shared = @_[0]; shift @_;
  my @a = split( '/', $shared); $shared = @a[$a-1];
  my @objs;
  my $i = 0;
  while(@_) {
    if (@_[0] eq ";") { shift @_; last; }
    @a = split( '/', @_[0]); my $src = @a[$a-1];
    if ($src =~ /(\w+)\.asm$/ ) {
      push @objs, " \$(LIB_SO_OUTPUT_DIR)$1\_so.\$(O)";
    }
    else {
      push @objs, " \$(LIB_SO_OUTPUT_DIR)$src\_so.\$(O)";
    }
    $i++;
    if (($i&3)==3) { push @objs, "\\\n  "; }
    shift @_;
  }
  print OUT "$shared.so: @objs\n";
  print OUT "\t\$(LDSO) \$(OUT) \$(LIB_SO_OUTPUT_DIR)$shared.\$(SO) \\\n";
  print OUT "\t @objs\\\n\t ";
  while(@_[0]) {
    my $link = @_[0];
    print OUT "\$(LIBINC)$link ";
    shift @_;
  }
  print OUT "\\\n\t \$(LDSO_FLAGS)\n";
  print OUT "\n\n";
}

sub is_excluded
{
  if (@Exclude{@_[0]}==1) { return 1; }
  return 0;
}

sub write_lib_entry   # $dir, $lib, $Nb
{
  my $dir = @_[0];
  my $lib = @_[1];
  my $Nb  = @_[2];

  if (not -d "$dir") {
    die( "Directory $dir does not exist\n" );
  }
  opendir (DIR, $dir) || die ("Cannot opendir $dir\n");

  my @cpp_objs_cond;
  my @asm_objs_cond;

  while ($file=readdir(DIR)) {
    if ($file =~ /(\w+)\.cpp$/) {
      if (@Conditional_Flag{"$1.cpp"}) {
        push(@cpp_objs_cond, $1);
        next;
      }
      if (@Exclude{"$1.cpp"}==1) {
        print "    excluding file '$1.cpp'\n";
        next;
      }
      push(@cpp_objs,$1);
    }
    elsif ($file =~ /(\w+)\.asm$/) { 
      if (@Conditional{"$1.asm"}) { 
        my $flag = @Conditional_Flag{"$1.asm"};
        my $val = @Conditional_Value{"$1.asm"};
        push(@asm_objs_cond, $1);
        next;
      }
      if (@Exclude{"$1.asm"}==1) {
        print "    excluding file '$1.asm'\n";
        next;
      }
      push(@asm_objs,$1);
    }
    elsif ($file eq ".") { next; }
    elsif ($file eq "..") { next; }
    elsif ($file =~ /.+.h/) { next; }
    else { print "     ignoring file '$file'\n"; }
  }
  closedir DIR;

  my $flag = @Libs_Flags{$lib};
  if ($flag) {
    if ($flag =~ /!(\w+)/) {
      print OUT "ifndef $1\n\n";
    }
    else {
      print OUT "ifdef $flag\n\n";
    }
  }
        ##### regular cpp compilation #####

  my $name_OBJ = "$lib\_OBJ$Nb";

  print OUT "$name_OBJ = ";
  my @objs = @cpp_objs;
  while(@objs) {
    print OUT "\\\n";
    print OUT "  \$(OBJ_OUTPUT_DIR)@objs[0].\$(O)\t\t";
    shift @objs;
  }
  print OUT "\n\n";

        ##### conditional cpp compilation #####

  while(@cpp_objs_cond) {
    my $obj = @cpp_objs_cond[0];
    my $f = "$obj.cpp";
    push(@cpp_objs, $obj);
    print OUT "ifeq (\$(@Conditional_Flag{$f}),@Conditional_Value{$f})\n";
    print OUT "$name_OBJ += \$(OBJ_OUTPUT_DIR)$obj.\$(O)\n";
    print OUT "endif\n";
    shift @cpp_objs_cond;
  }
  print OUT "\n";

        ##### regular asm compilation #####

  my $name_ASM = "$lib\_OBJ_ASM$Nb";

  print OUT "ifdef USE_ASM\n";

  print OUT "$name_ASM = ";
  my @objs = @asm_objs;
  while(@objs) {
    print OUT "\\\n";
    print OUT "  \$(OBJ_OUTPUT_DIR)@objs[0].\$(O)\t\t";
    shift @objs;
  }
  print OUT "\n";

        ##### conditional asm compilation #####

  while(@asm_objs_cond) {
    my $obj = @asm_objs_cond[0];
    my $f = "$obj.asm";
    push(@asm_objs, $obj);
    print OUT "ifeq (\$(@Conditional_Flag{$f}),@Conditional_Value{$f})\n";
    print OUT "$name_ASM += \$(OBJ_OUTPUT_DIR)$obj.\$(O)\n";
    print OUT "endif\n";
    shift @asm_objs_cond;
  }
  print OUT "\nendif\n";
  print OUT "\n\n";

  if ($flag) {
    print OUT "\nendif   #$flag\n";
  }
}

#########################################

sub write_header
{
  print OUT "################################################\n";
  print OUT "#   Common makefile. Automatically generated.  #\n";
  print OUT "################################################\n";

  print OUT "\n";
  my @labels = values %libs_label;
  print OUT "all: @labels\n";
  print OUT "all_so: @Shared_Bin_So ";
  print OUT join("_so ", @labels); print OUT "_so\n";
  print OUT "\n";

  print OUT "ALL_TESTS = @Ex_Bin @Shared_Bin_So\n";
  print OUT "ALL_TESTS_BENCH =";
  my @exs = @Ex_Bin;
  while(@exs) {
    my $ex = @exs[0]; shift @exs;
    print OUT " $ex.test";
  }
  print OUT "\n";
  print OUT "ALL_TESTS_PURE =";
  my @exs = @Ex_Bin;
  while(@exs) {
    my $ex = @exs[0]; shift @exs;
    print OUT " $ex.pure";
  }
  print OUT "\n";

  print OUT "\n";
  print OUT "tests: \$(ALL_TESTS)\n";
  print OUT "tests.test: \$(ALL_TESTS_BENCH)\n";
  print OUT "tests.pure: \$(ALL_TESTS_PURE)\n";
  print OUT "\n";
}

#########################################
##        generation of libs           ##
#########################################

sub write_libs 
{
  print "-= LIBS =-\n";
  print OUT "#########################################\n";
  print OUT "##            LIBRARIES                ##\n";
  print OUT "#########################################\n";
  print OUT "\n";

  while(@libs) {
  
    my $lib = @libs[0];

    print "  [$lib]\n";

    print OUT "#############\n";
    print OUT "############# lib: $lib ################\n";
    print OUT "#############\n";
    print OUT "\n";

    my @lib_dirs = split( ',', @Libs_src_dirs{$lib} );
    my $Nb = 1;
    while(@lib_dirs)
    {    
      my $subdir = @lib_dirs[0];
      my $dir = "$src_dir/$subdir";
      my $dir_str = "\$($lib\_SRC_DIR$Nb)";

      print "   ($subdir)\n";

      print OUT "   ###### ($dir) ######\n";
      print OUT "\n";

      write_lib_entry($dir, $lib, $Nb);

      print OUT "$lib\_SRC_DIR$Nb = \$(LIB_SRC_DIR)$subdir\$(SEP)\n";
      print OUT "\n";

      print OUT "     ##### cpp objs for '$lib/$subdir'\n";
      print OUT "\n";


      while(@cpp_objs) {
        write_cpp_entry($dir_str,  @cpp_objs[0], "");  # no flag?
        shift @cpp_objs;
      }
      while (@asm_objs) {
        print OUT "     ##### asm objs for '$lib/$subdir'\n";
        print OUT "\n";
        while(@asm_objs) {
          write_asm_entry($dir_str, @asm_objs[0], "_a");  # no flag?
          shift @asm_objs;
        }
      }

      print OUT "\n";
      shift @lib_dirs;
      $Nb++;
    }
    print OUT "  ##### main entry for '$lib' #####\n";
    print OUT "\n";

          ##### lib entry #####

    my $label = @libs_label{$lib};
    my $name = "LIB_$lib";
    my $name_so = "LIB_$lib\_SO";

    print OUT "$name\_ALL_OBJS = ";
    while ($Nb>1) {
      $Nb--;
      print OUT " \$($lib\_OBJ$Nb) \$($lib\_OBJ_ASM$Nb)";
    }
    print OUT "\n\n";

    print OUT "LIB_$lib    = \$(LIB_OUTPUT_DIR)libskl_$label.\$(A)\n";
    print OUT "LIB_$lib\_SO = \$(LIB_SO_OUTPUT_DIR)libskl_$label.\$(SO)\n";

    print OUT "\$($name): \$($name\_ALL_OBJS)\n";
    print OUT "\t\$(AR) \$($name) \$($name\_ALL_OBJS)\n";
    print OUT "\t\$(RANLIB) \$($name)\n";
    print OUT "\$($name\_SO): \$($name\_ALL_OBJS)\n";
    print OUT "\t\$(LDSO) \$(OUT) \$($name\_SO) \$($name\_ALL_OBJS) \\\n\t";
    if (@Libs_so_link{$lib}) {
      my @Links = split( ' ', @Libs_so_link{$lib}); 
      while(@Links) {
        print OUT "\$(LIBINC)$Links[0] ";
        shift @Links;
      }
    }
    print OUT "\$(LDSO_FLAGS)\n\n";

    print OUT "$label:    \$($name\)\n";
    print OUT "$label\_so: \$($name_so)\n";
    print OUT "\n\n";

    shift @libs;
  }
}

#########################################
##    generation of shared objects     ##
#########################################

sub write_shared
{
  print "-= SHARED =-\n";
  print OUT "#########################################\n";
  print OUT "##            SHARED OBJ               ##\n";
  print OUT "#########################################\n";
  print OUT "\n";

  while(@Shared_Bin) {
    my @Src;
    while(@Shared_Src) {
      if (@Shared_Src[0] eq ";") { shift @Shared_Src; last; }
      push( @Src, @Shared_Src[0] );
      shift @Shared_Src;
    }

    my @Flag;
    while(@Shared_Flag) {
      if (@Shared_Flag[0] eq ";") { shift @Shared_Flag; last; }
      push( @Flag, @Shared_Flag[0] );
      shift @Shared_Flag;
    }
    my @Link;
    while(@Shared_Link) {
      if (@Shared_Link[0] eq ";") { shift @Shared_Link; last; }
      push( @Link, @Shared_Link[0] );
      shift @Shared_Link;
    }
    print "  [@Shared_Bin[0]] ( ";
    print OUT "## Shared obj: @Shared_Bin[0]\n\n";
    my @src = @Src;
    while(@src) {
      if (@src[0] =~ /([\w\/]+)\.asm/) { 
        write_asm_so_entry( $1, "_so", @Flag );
        print "$1.asm ";
      }
      elsif (@src[0] =~ /([\w\/]+)\.cpp/) { 
        write_cpp_so_entry($1, "_so", @Flag, "\$(CCSO_FLAGS)"); 
        print "$1.cpp ";
      }
      else { 
        write_cpp_so_entry(@src[0], "_so", @Flag, "\$(CCSO_FLAGS)");
        print "@src[0].cpp ";
      }
      shift @src;
    }
    print")\n";
    write_so_entry(@Shared_Bin[0], @Src, ";", @Link);
    shift @Shared_Bin;
  }
}

#########################################
##       generation of examples        ##
#########################################

sub write_ex_entry
{  
  my $exe = @_[0]; shift @_;
  my @objs;
  while(@_) {
    if (@_[0] eq ";") { shift @_; last; }
    push @objs, " \$(OBJ_OUTPUT_DIR)@_[0].\$(O)";
    shift @_;
  }
  my @libs;
  my @deps;
  while(@_) {
    push @libs, "\$(LIBINC)skl\_@_[0] ";
    push @deps, "\\\n\t  \$(LIB_OUTPUT_DIR)libskl\_@_[0].\$(A) ";
    shift @_;
  }

  print OUT "$exe: @objs @deps\n";
  print OUT "\t\$(LD) \$(OUT) \$(BUILD_DIR)$exe \\\n";
  print OUT "\t @objs\\\n";
  print OUT "\t \$(LIBDEF)\$(LIB_OUTPUT_DIR) @libs \$(LD_FLAGS)\n";
  print OUT "\n\n";

  print OUT "$exe.pure: @objs @deps\n";
  print OUT "\t\$(PURIFY) \$(LD) \$(OUT) \$(BUILD_DIR)$exe.pure \\\n";
  print OUT "\t @objs\\\n";
  print OUT "\t \$(LIBDEF)\$(LIB_OUTPUT_DIR) @libs \$(LD_FLAGS)\n";
  print OUT "\n\n";

  print OUT "$exe.test: $exe\n";
  print OUT "\t.\$(SEP)$exe > test\$(SEP)$exe.test\n";
  print OUT "\t\$(DIFF) test\$(SEP)$exe.test \\\n";
  print OUT "\t  ..\$(SEP)..\$(SEP)data\$(SEP)$exe.test > test\$(SEP)$exe.diff\n";
  print OUT "\trm .\$(SEP)$exe\n";
  print OUT "\n\n";
}

sub write_examples
{
  print "-= EXAMPLES =-\n";
  print OUT "#########################################\n";
  print OUT "##            EXAMPLES                 ##\n";
  print OUT "#########################################\n";
  print OUT "\n";

  while(@Ex_Bin) {
    my @Src;
    while(@Ex_Src) {
      if (@Ex_Src[0] eq ";") { shift @Ex_Src; last; }
      push( @Src, @Ex_Src[0] );
      shift @Ex_Src;
    }
    if (not @Src) { push(@Src, @Ex_Bin[0]); }

    my @Dep;
    while(@Ex_Dep) {
      if (@Ex_Dep[0] eq ";") { shift @Ex_Dep; last; }
      push( @Dep, @Ex_Dep[0] );
      shift @Ex_Dep;
    }

    my @Flag;
    while(@Ex_Flag) {
      if (@Ex_Flag[0] eq ";") { shift @Ex_Flag; last; }
      push( @Flag, @Ex_Flag[0] );
      shift @Ex_Flag;
    }
    if (@Ex_Type[0]=="0") { $src_dir = "TEST_SRC_DIR"; }
    elsif (@Ex_Type[0]=="1") { $src_dir = "EXAMPLE_SRC_DIR"; }
    shift @Ex_Type;

    print "  [@Ex_Bin[0]] ( ";
    print OUT "## Example: @Ex_Bin[0]\n\n";
    my @src = @Src;
    while(@src) { 
      if (@src[0] =~ /(\w+)\.asm/) { 
        write_asm_entry("\$($src_dir)", $1, "_a", @Flag);
        print "$1.asm ";
      }
      elsif (@src[0] =~ /(\w+)\.cpp/) { 
        write_cpp_entry("\$($src_dir)", $1, "", @Flag); 
        print "$1.cpp ";
      }
      else { 
        write_cpp_entry("\$($src_dir)", @src[0], "", @Flag);
        print "@src[0].cpp ";
      }
      shift @src;
    }
    print")\n";
    write_ex_entry(@Ex_Bin[0],@Src,";", @Dep);
    shift @Ex_Bin;
  }
}

#########################################

sub write_makefile
{
  print "Generating $makefile\n";
  open(OUT,">$makefile") || die( "can't open $makefile\n" );
  write_header;
  write_libs;
  write_shared;
  write_examples;
  close OUT;
}

#########################################
##      Doc entries (files.txt)        ##
#########################################

sub write_doc
{
  print "-= DOC ($doctxt) =-\n";
  open(DOC,">$doctxt") || die( "can't open $doctxt\n" );
    push(@include_dir, $inc_dir);
    while(@include_dir) {
      my $file;
      my $dir = @include_dir[0];
      print "  [$dir]\n";
      shift @include_dir;
      opendir (DIR, $dir) || die ("Cannot opendir $dir\n");
      while ($file=readdir(DIR)) {
        if ($file =~ /(\w+).h$/) { push(@h_objs,"$dir/$1.h"); }
      }
      closedir DIR;
    }

    while(@h_objs) {
      print DOC "//\@Include: @h_objs[0]\n";
      shift @h_objs;
    }

  close DOC;
}

#########################################
##               main                  ##
#########################################

read_config_file;
write_makefile;
write_doc;

#########################################
