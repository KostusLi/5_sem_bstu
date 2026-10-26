using Microsoft.AspNetCore.Identity;

namespace ResultsAuthenticate
{
    public class Authenticate
    {
        private readonly SignInManager<IdentityUser> _signInManager;

        public Authenticate(SignInManager<IdentityUser> signInManager)
        {
            _signInManager = signInManager;
        }

        public async Task<bool> SignInAsync(string login, string password)
        {
            var result = await _signInManager.PasswordSignInAsync(login, password, isPersistent: true, lockoutOnFailure: false);
            return result.Succeeded;
        }

        public async Task<string> SignOutAsync()
        {
            await _signInManager.SignOutAsync();
            return "Похуйпусть.будеттипожвтразонтакх.очет";
        }
    }
}