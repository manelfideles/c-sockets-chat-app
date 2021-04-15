if {$argc == 8} {
	set Size [lindex $argv 0]
	set Speed1 [lindex $argv 1]
	set Tp1 [lindex $argv 2]
	set Speed2 [lindex $argv 3]
	set Tp2 [lindex $argv 4]
	set Speed3 [lindex $argv 5]
	set Tp3 [lindex $argv 6]
	set End [lindex $argv 7]
} else {
	puts "Usage: ns link_simulation_2.tcl {size} {L1} {Tp1} {L2} {Tp2} {L3} {Tp3} {simulation end}"
	exit 1
}

set ns [new Simulator]

set nf [open out.nam w]
$ns namtrace-all $nf

proc fim {} {
	global ns nf
	$ns flush-trace
	close $nf
	exec nam out.nam
	exit 0
	}

#rede 1
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
$ns duplex-link $n0 $n1 $Speed1 $Tp1 DropTail
$ns duplex-link $n1 $n2 $Speed2 $Tp2 DropTail
$ns duplex-link $n2 $n3 $Speed3 $Tp3 DropTail
$ns duplex-link-op $n0 $n1 queuePos 0.5
$ns duplex-link-op $n1 $n2 queuePos 0.5
$ns duplex-link-op $n2 $n3 queuePos 0.5

$ns queue-limit $n0 $n1 20480

set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ $Size	;# em bytes
$cbr0 set interval_ 1		;# não é preciso porque só enviamos 1
$cbr0 set maxpkts_ 1
$cbr0 attach-agent $udp0

set null1 [new Agent/Null]
$ns attach-agent $n3 $null1

$ns connect $udp0 $null1

$ns at 0.1 "$cbr0 start"
$ns at $End "$cbr0 stop"

$ns at $End "fim"

$ns run
