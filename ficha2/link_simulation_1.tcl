if {$argc == 4} {
	set Size [lindex $argv 0]
	set Speed [lindex $argv 1]
	set Tp [lindex $argv 2]
	set End [lindex $argv 3]
} else {
	puts "Usage: ns link_simulation_1.tcl {size} {Link speed} {Propagation time} {simulation end}"
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

set n0 [$ns node]
set n1 [$ns node]
$ns duplex-link $n0 $n1 $Speed $Tp DropTail
$ns queue-limit $n0 $n1 20480

$ns duplex-link-op $n0 $n1 queuePos 0.5

set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ $Size	;# em bytes
$cbr0 set interval_ 1		;# não é preciso porque só enviamos 1
$cbr0 set maxpkts_ 1
$cbr0 attach-agent $udp0

set null1 [new Agent/Null]
$ns attach-agent $n1 $null1

$ns connect $udp0 $null1

$ns at 0.1 "$cbr0 start"
$ns at $End "$cbr0 stop"

$ns at $End "fim"

$ns run
