{if $error != ''}
	<p>{$error}</p>
{/if}

<div id="login_leftcol">
	<form method="post" action="{$base_url}/app/login/" id="login_form">
	<fieldset>
		<legend>Login</legend>
		<div>
			<label for="username">Username:</label>
			<input type="text" name="username" id="username" />
		</div>
		<div>
			<label for="password">Password:</label>
			<input type="password" name="password" id="password" />
		</div>
		<div>
			<input type="submit" name="do_login" id="do_login" value="Login" />
		</div>
	</fieldset>
	</form>
</div>
<div id="login_rightcol">
	<p>If you need login access to this system, contact <a href="http://robert.accettura.com/contact/?subject=Reporter%20Access%20Request">Robert Accettura</a>.</p>
	<p><strong>Note:</strong> Login Access is only given in special circumstances.</p>
</div>
<br style="clear: both;" />