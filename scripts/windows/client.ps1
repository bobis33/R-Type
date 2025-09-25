$udp = New-Object System.Net.Sockets.UdpClient
$msg = [System.Text.Encoding]::UTF8.GetBytes("Hello server")
$udp.Send($msg, $msg.Length, "127.0.0.1", 2560)
$udp.Close()
Write-Host "Message sent"
