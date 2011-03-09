$cwd=`cd`;
chomp($cwd);
$cwd =~ s/\\/\\\\\\\\/g;

while(<>) {
  $_ =~ s/$cwd/[TARGETDIR]/g;
  $_ =~ s/\[TARGETDIR\]\\\\..\\\\../[TARGETDIR]/g;
  $_ =~ s/C:\\\\Windows\\\\system32\\\\/[SystemFolder]/g;
  print $_;
}

