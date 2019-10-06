while($true){$data = $(Invoke-Expression $cmd); $reading = $($data | ConvertFrom-Json | ConvertTo-HashTable).properties.reported.GasSensor;$timestamp=$($data | ConvertFrom-Json | ConvertTo-HashTable).properties.reported.'$metadata'.'$lastUpdated';Write-Output "$timestamp,$reading" | Out-File temp.txt -Append}



